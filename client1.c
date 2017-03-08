#include <unp.h>

#define PORT 9877
#define MAXBUF 8192

int main(int argc, char const *argv[])
{
	struct sockaddr_in seraddr;
	int sockfd;
	char buf[MAXBUF];

	sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	bzero(&seraddr, sizeof(seraddr));
	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(PORT);
	Inet_pton(AF_INET, argv[1], &seraddr.sin_addr);

	Connect(sockfd, (SA*)&seraddr, sizeof(seraddr));

	while (Fgets(buf, MAXBUF, stdin) != NULL)
	{
		Write(sockfd, buf, strlen(buf));
	}

	exit(0);
}