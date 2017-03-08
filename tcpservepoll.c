/*
typedef union epoll_data{
	void *ptr;
	int fd;
	uint32_t u32;
	uint64_t u64;
} epoll_data_t;

struct epoll_event {
	uint32_t events;
	epoll_data_t data;
}

int epoll_create(int size);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
op:
EPOLLIN
EPOLLOUT
EPOLLPRI
EPOLLERR
EPOLLHUP
EPOLLET
EPOLLONESHOT
int epoll_wait(int epfd, strut epoll_event *events, int maxevents, int timeout);
*/

#include <sys/socket.h>	//for socket
#include <netinet/in.h>
#include <stdio.h>	//for perror
#include <stdlib.h>	//for exit
#include <string.h>	//for bzero
#include <sys/epoll.h>	//for epoll

#define PORT 9877
#define MAXSIZE 1000
#define MAXEVENTS 100
#define LISTENQ 100
#define MAXBUF 8192

int Socket(int family, int type, int protocol);
int Bind(int fd, struct sockaddr *addr, socklen_t len);
int Listen(int fd, int backlog);
int Epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
const char *Sock_ntop(const struct sockaddr * addr, socklen_t len);
int Close(int fd);
int AddEvent(int epfd, int fd, int state);
int DelEvent(int epfd, int fd, int state);
int ModEvent(int epfd, int fd, int state);
void do_read(int epfd, int fd, char *buf);

int main(int argc, char const *argv[])
{
	int listenfd, epfd, on = 1;
	struct sockaddr_in cliaddr, servaddr;
	struct epoll_event events[MAXEVENTS];
	char buf[MAXBUF];

	//set server addr
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;	//IPv4
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	listenfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	epfd = epoll_create(MAXSIZE);
	AddEvent(epfd, listenfd, EPOLLIN);

	//epoll wait
	for (; ;)
	{
		int waitn, i, fd;
		waitn = Epoll_wait(epfd, events, MAXEVENTS, -1);

		for (i = 0; i < waitn; ++i)	//for each trigger event
		{
			fd = events[i].data.fd;
			if (fd == listenfd && (events[i].events & EPOLLIN))
			{
				//accept
				socklen_t len = sizeof(cliaddr);
				fd = accept(listenfd, (struct sockaddr *)&cliaddr, &len);	//new client
				printf("Accept new client: %s\n", Sock_ntop((struct sockaddr *)&cliaddr, len));
				AddEvent(epfd, fd, EPOLLIN);	//add
			}
			else if (events[i].events == EPOLLIN)
			{
				//read
				do_read(epfd, fd, buf);
			}
			else if (events[i].events == EPOLLOUT)
			{
				//write
			}
		}
	}
	Close(epfd);

	exit(0);
}

int Socket(int family, int type, int protocol)
{
	int fd;
	if ((fd = socket(family, type, protocol)) < 0)
	{
		perror("socket error: ");
		exit(1);
	}
	return fd;
}

int Bind(int fd, struct sockaddr *addr, socklen_t len)
{
	int ret;
	if ( (ret = bind(fd, addr, len))  < 0 )
	{
		perror("bind error: ");
		exit(1);
	}
	return ret;
}

int Listen(int fd, int backlog)
{
	int ret;
	if ((ret = listen(fd, backlog)) < 0)
	{
		perror("listen error");
		exit(1);
	}
	return ret; 
}

int AddEvent(int epfd, int fd, int state)
{
	int ret;

	struct epoll_event event;
	event.events = state;
	event.data.fd = fd;

	if ((ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event)) < 0)
	{
		perror("epoll_ctl error: ");
		exit(1);
	}
	else printf("epoll_ctl succeed for %d\n",  fd);
	return ret;
}

int DelEvent(int epfd, int fd, int state)
{
	int ret;

	struct epoll_event event;
	event.events = state;
	event.data.fd = fd;

	if ((ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &event)) < 0)
	{
		char emsg[1024];
		snprintf(emsg, sizeof(emsg), "epoll_ctl error [args: epfd  = %d, fd = %d, state = %d] at %d of %s",
			epfd, fd, state, __LINE__, __FILE__);
		perror(emsg);
		exit(1);
	}
	return ret;
}

int ModEvent(int epfd, int fd, int state)
{
	int ret;

	struct epoll_event event;
	event.events = state;
	event.data.fd = fd;

	if ((ret = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event)) < 0)
	{
		perror("epoll_ctl error: ");
		exit(1);
	}
	return ret;
}

int Epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	int ret;
	if ((ret = epoll_wait(epfd, events, maxevents, timeout)) < 0)
	{
		perror("epoll errr: ");
		exit(1);
	}
	return ret;
}

int Close(int fd)
{
	int ret;
	if ((ret = close(fd)) < 0)
	{
		perror("close error: ");
		exit(1);
	}
	return ret;
}

const char *Sock_ntop(const struct sockaddr * addr, socklen_t len)
{
	char port[8];
	static char str[128];

	switch (addr->sa_family)
	{
		struct sockaddr_in *sin;
	case AF_INET:
		sin = (struct sockaddr_in *)&addr;
		if (!inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)))
			return NULL;
		if (ntohs(sin->sin_port) != 0)
		{
			snprintf(port, sizeof(port), ":%d", ntohs(sin->sin_port));
			strcat(str, port);
		}
		return str;
	}
}

void do_read(int epfd, int fd, char *buf)
{
	printf("do_read\n");

	int nread;
	while ( (nread = read(fd, buf, MAXBUF)) > 0)
	{
		buf[nread] = 0;
		printf("Receive message: %s\n", buf);
		ModEvent(epfd, fd, EPOLLOUT);
	}

	if (nread == -1)
	{
		perror("read error");	
		DelEvent(epfd, fd, EPOLLIN);
		Close(fd);
	}
	else if(nread == 0)
	{
		fprintf(stderr, "client close\n");	
		DelEvent(epfd, fd, EPOLLIN);
		Close(fd);
	}
}