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

#define LISTEN_BACKLOG 50

static int tryconnectserver(serverinfo *info)
{
	int sockfd = -1;
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		handle_warning("socket");
		return -1;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(info->port);
	inet_pton(sockfd, info->ip, &server_addr.sin_addr.s_addr);

	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		close(sockfd);
		handle_warning("socket");
		return -1;
	}

	return sockfd;
}

static void connectserver(int *confd, unsigned int *serverid)
{
	int back = 0;
	int sockfd = -1;
	message *sendmsg = NULL;
	message *recvmsg = NULL;
	message *backmsg = NULL;
	serverinfo info;
	struct sockaddr_in server_addr;

	if (!confd || !serverid)
		return;

	*confd = -1;

	sendmsg = malloc(sizeof(message));
	if (!sendmsg)
	{
		handle_warning("malloc");
		return;
	}

	recvmsg = malloc(sizeof(message) + sizeof(serverinfo));
	if (!recvmsg)
	{
		free(sendmsg);
		handle_warning("malloc");
		return;
	}

	backmsg = malloc(sizeof(message) + sizeof(int) + sizeof(unsigned int));
	if (!backmsg)
	{
		free(recvmsg);
		free(sendmsg);
		handle_warning("malloc");
		return;
	}

	memset(sendmsg, 0, sizeof(message));
	memset(recvmsg, 0, sizeof(message) + sizeof(serverinfo));
	memset(backmsg, 0, sizeof(message) + sizeof(int) + sizeof(unsigned int));
	memset(&info, 0, sizeof(serverinfo));
	memset(&server_addr, 0, sizeof(server_addr));

	sendmsg->cmd = cmd_connect;
	sendmsg->arglen = 0;

	backmsg->cmd = cmd_backconnect;
	backmsg->arglen = sizeof(int) + sizeof(unsigned int);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		free(sendmsg);
		free(recvmsg);
		handle_warning("socket");
		return;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(DIS_PORT);
	inet_pton(sockfd, DIS_IP, &server_addr.sin_addr.s_addr);

	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		free(sendmsg);
		free(recvmsg);
		close(sockfd);
		handle_warning("socket");
		return;
	}
	while (1)
	{
		writen(sockfd, sendmsg, sizeof(message));
		if (readn(sockfd, recvmsg, sizeof(message) + sizeof(serverinfo)) == 0)
		{
			free(sendmsg);
			free(recvmsg);
			close(sockfd);
			handle_warning("readn");
			return;
		}

		memcpy(&info, recvmsg->argv, sizeof(serverinfo));
		*confd = tryconnectserver(&info);
		if (*confd > 0)
		{
			*serverid = info.id;
			//���������������
			back = 1;
			memcpy(backmsg->argv, &back, sizeof(int));
			memcpy(backmsg->argv + sizeof(int), &info.id, sizeof(unsigned int));
			writen(sockfd, backmsg, sizeof(message) + sizeof(int));
			break;
		}

		//������������У��ٸ���һ��
		back = 0;
		memcpy(backmsg->argv, &back, sizeof(int));
		memcpy(backmsg->argv + sizeof(int), &info.id, sizeof(unsigned int));
		writen(sockfd, backmsg, sizeof(message) + sizeof(int));

		sleep(30);
	}

	free(sendmsg);
	free(recvmsg);
	close(sockfd);
	return;
}

static void unconnectserver(int confd, unsigned int serverid)
{
	close(confd);

	int sockfd = -1;
	message *sendmsg = NULL;
	struct sockaddr_in server_addr;

	sendmsg = malloc(sizeof(message) + sizeof(unsigned int));
	if (!sendmsg)
	{
		handle_warning("malloc");
		return;
	}

	memset(sendmsg, 0, sizeof(message));
	memset(&server_addr, 0, sizeof(server_addr));

	sendmsg->cmd = cmd_unconnect;
	sendmsg->arglen = sizeof(unsigned int);
	memcpy(sendmsg->argv, &serverid, sendmsg->arglen);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		handle_warning("socket");
		return;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(DIS_PORT);
	inet_pton(sockfd, DIS_IP, &server_addr.sin_addr.s_addr);

	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		close(sockfd);
		handle_warning("socket");
		return;
	}

	writen(sockfd, sendmsg, sizeof(message) + sizeof(unsigned int));
}

int main(int argc, char ** argv)
{
    int i = 0;
	int sockfd = -1;
	unsigned int serverid = 0;
	ssize_t readret = 0;

	char read_buff[256] = { 0 };
	char write_buff[256] = { 0 };

	connectserver(&sockfd, &serverid);
	if (sockfd < 0)
		handle_error("connectserver");

	for (i = 0; i < 10; ++i)
	{
		memset(write_buff, 0, sizeof(write_buff));
		sprintf(write_buff, "This is client send message:%d", i);
		write(sockfd, write_buff, strlen(write_buff) + 1);

		memset(read_buff, 0, sizeof(read_buff));
		readret = read(sockfd, read_buff, sizeof(read_buff));
		if (readret == 0)
			break;
		printf("%s\n", read_buff);

		sleep(1);
	}

	unconnectserver(sockfd, serverid);

    return 0;
}
