#ifndef WRAPPER_H__
#define WRAPPER_H__

#include <sys/socket.h>	//for socket
#include <netinet/in.h>
#include <stdio.h>	//for perror
#include <stdlib.h>	//for exit
#include <string.h>	//for bzero
#include <sys/epoll.h>	//for epoll

#define SA struct sockaddr
#define LISTENQ 1024
#define MAXEVENT 100
#define MAXSIZE 1000
#define PORT 9877
#define BUFFER_SIZE 1024
#define MAXBUF 8192

int Socket(int family, int type, int protocol)
{
	int fd;
	if ((fd = socket(family, type, protocol)) < 0)
	{
		perror("socket error");
		exit(1);
	}
	return fd;
}

int Bind(int fd, struct sockaddr *addr, socklen_t len)
{
	int ret;
	if ( (ret = bind(fd, addr, len))  < 0 )
	{
		perror("bind error");
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
		perror("epoll_ctl error");
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
		perror("epoll_ctl error");
		exit(1);
	}
	return ret;
}

int Epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	int ret;
	if ((ret = epoll_wait(epfd, events, maxevents, timeout)) < 0)
	{
		perror("epoll errr");
		exit(1);
	}
	return ret;
}

int Close(int fd)
{
	int ret;
	if ((ret = close(fd)) < 0)
	{
		perror("close error");
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

#endif