#include <unp.h>

int main(int argc, char const *argv[])
{
	// socket connect
	int sockfd;
	struct  sockaddr_in servaddr;

	if (argc != 2)
		err_quit("usag: tcpcli <IPAddress>");

	sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	Connect(sockfd, (SA*) &servaddr, sizeof(servaddr));

	str_cli(stdin, sockfd);

	exit(0);
}
