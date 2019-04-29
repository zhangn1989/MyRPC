#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>

#include "public_head.h"
#include "fileio.h"

#define LISTEN_BACKLOG	50
#define ADD_IP		"127.0.0.1"
#define ADD_PORT	10086
#define MUL_IP		ADD_IP
#define MUL_PORT	10010

int main(int argc, char ** argv)
{
	int sockfd = 0;
	Message addMessage, mulMessage;
	struct sockaddr_in addServer_addr;
	struct sockaddr_in mulServer_addr;

	memset(&addServer_addr, 0, sizeof(addServer_addr));
	memset(&mulServer_addr, 0, sizeof(mulServer_addr));
	memset(&addMessage, 0, sizeof(Message));
	memset(&mulMessage, 0, sizeof(Message));

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		handle_error("socket");

	addServer_addr.sin_family = AF_INET;
	addServer_addr.sin_port = htons(ADD_PORT);
	inet_pton(sockfd, ADD_IP, &addServer_addr.sin_addr.s_addr);

	addMessage.arg1 = 3;
	addMessage.arg2 = 5;
	if (connect(sockfd, (struct sockaddr*) & addServer_addr, sizeof(addServer_addr)) < 0)
	{
		close(sockfd);
		handle_error("connect");
	}

//	while (1)
	{
		write(sockfd, &addMessage, sizeof(Message));
		read(sockfd, &addMessage, sizeof(Message));
		printf("%d + %d = %d\n", addMessage.arg1, addMessage.arg2, addMessage.result);
// 		if (readret == 0)
// 			break;
		sleep(1);
	}
	close(sockfd);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		handle_error("socket");

	mulServer_addr.sin_family = AF_INET;
	mulServer_addr.sin_port = htons(MUL_PORT);
	inet_pton(sockfd, MUL_IP, &mulServer_addr.sin_addr.s_addr);

	mulMessage.arg1 = 3;
	mulMessage.arg2 = 5;
	if (connect(sockfd, (struct sockaddr*) & mulServer_addr, sizeof(mulServer_addr)) < 0)
	{
		close(sockfd);
		handle_error("connect");
	}

//	while (1)
	{
		write(sockfd, &mulMessage, sizeof(Message));
		read(sockfd, &mulMessage, sizeof(Message));
		printf("%d x %d = %d\n", mulMessage.arg1, mulMessage.arg2, mulMessage.result);
// 		if (readret == 0)
// 			break;
		sleep(1);
	}
	close(sockfd);

	return 0;
}
