#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>

#include "public_head.h"

#define LISTEN_BACKLOG	50
#define THREAD_COUNT	3
#define WEIGHT_BOTTOM	1
#define WEIGHT_TOP		1024
#define WEIGHT_ADD(w)	{if((w) <= (WEIGHT_TOP / 2)) (w) *= 2;}
#define WEIGHT_SUB(w)	{if((w) > WEIGHT_BOTTOM) (w) /= 2;}

typedef struct __register_server
{
	int weight;
	int client_count;
	serverinfo info;
}register_server;

static int clientfd[QUEUE_MAX];
static int *client_start;
static int *client_end;

static register_server serverlist[QUEUE_MAX];

static pthread_mutex_t servermutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void cmd_connect_func(int acceptfd, unsigned char *argv, int arglen)
{
	int i = 0;
	int id = 0;
	int sub = 0;
	int maxsub = 0;
	message *sendmsg = NULL;

	sendmsg = malloc(sizeof(message) + sizeof(serverinfo));
	if (!sendmsg)
	{
		handle_warning("malloc");
		return;
	}

	memset(sendmsg, 0, sizeof(message) + sizeof(serverinfo));

	//用负载均衡算法找出最优服务器，并发送给请求客户端
	for (i = 0; i < QUEUE_MAX; ++i)
	{
		if(serverlist[i].info.id < 0)
			continue;

		sub = serverlist[i].weight - serverlist[i].client_count;
		if (sub > maxsub)
		{
			id = i;
			maxsub = sub;
		}
	}

	memcpy(sendmsg->argv, &serverlist[id].info, sizeof(serverinfo));
	writen(acceptfd, sendmsg, sizeof(message) + sizeof(serverinfo));
}

void cmd_backconnect_func(int acceptfd, unsigned char *argv, int arglen)
{
	connectback cb;
	int back = 0;
	int id = 0;

	memcpy(&cb, argv, sizeof(connectback));
	back = cb.back;
	id = cb.id;
	if (back)
	{
		//给id服务器加权，增加连接数
		pthread_mutex_lock(&servermutex);
	//	WEIGHT_ADD(serverlist[cb.id].weight);
		{
			if ((serverlist[id].weight) <= (WEIGHT_TOP / 2))
				(serverlist[id].weight) *= 2;
		}
		serverlist[cb.id].client_count++;
		pthread_mutex_unlock(&servermutex);
	}
	else
	{
		//给id服务器减权，并重新发送新的服务器
		pthread_mutex_lock(&servermutex);
	//	WEIGHT_SUB(serverlist[cb.id].weight);
		{
			if ((serverlist[id].weight) > WEIGHT_BOTTOM)
				(serverlist[id].weight) /= 2;
		}
		pthread_mutex_unlock(&servermutex);
		cmd_connect_func(acceptfd, argv, arglen);
	}
}

void cmd_unconnect_func(int acceptfd, unsigned char *argv, int arglen)
{
	int id = 0;
	memcpy(&id, argv, sizeof(int));

	//找到对应id的服务器，对其进行减连接数操作
	pthread_mutex_lock(&servermutex);
	serverlist[id].client_count--;
	pthread_mutex_unlock(&servermutex);
}

void cmd_register_func(int acceptfd, unsigned char *argv, int arglen)
{
	
	int i;
	int id = 0;
	message *msg = NULL;

	pthread_mutex_lock(&servermutex);
	for (i = 0; i < QUEUE_MAX; ++i)
	{
		if (serverlist[i].info.id == -1)
		{
			memcpy(&serverlist[i].info, argv, sizeof(serverinfo));
			serverlist[i].info.id = i;
			break;
		}
	}
	pthread_mutex_unlock(&servermutex);

	if (i == QUEUE_MAX)
		id = -1;
	else
		id = i;

	msg = malloc(sizeof(message) + sizeof(int));
	if (msg)
	{
		msg->cmd = cmd_register;
		msg->arglen = sizeof(int);
		memcpy(msg->argv, &id, msg->arglen);
		writen(acceptfd, msg, sizeof(message) + sizeof(int));
		free(msg);
	}
	else
	{
		handle_warning("malloc");
	}

	return;
}

void cmd_unregister_func(int acceptfd, unsigned char *argv, int arglen)
{
	int id = 0;
	memcpy(&id, argv, arglen);

	//删除该服务器
	serverlist[id].client_count = 0;
	serverlist[id].weight = WEIGHT_TOP;
	memset(&serverlist[id].info, 0, sizeof(serverinfo));
	serverlist[id].info.id = -1;
}

void cmd_heart_func(int acceptfd, unsigned char *argv, int arglen)
{

}

static void handle_request(int acceptfd)
{
	message msg;
	ssize_t readret = 0;
	unsigned char *argv = NULL;
	while (1)
	{
		memset(&msg, 0, sizeof(message));
		readret = readn(acceptfd, &msg, sizeof(message));
		if (readret == 0)
		{
			close(acceptfd);
			return;
		}

		if (msg.arglen > 0)
		{
			argv = malloc(msg.arglen);
			if (!argv)
			{
				handle_warning("malloc");
				close(acceptfd);
				return;
			}

			readret = readn(acceptfd, argv, msg.arglen);
			if (readret == 0)
			{
				handle_warning("readn argv");
				free(argv);
				argv = NULL;
				close(acceptfd);
				return;
			}
		}

		switch (msg.cmd)
		{
		case cmd_connect:
			cmd_connect_func(acceptfd, argv, msg.arglen);
			break;
		case cmd_backconnect:
			cmd_backconnect_func(acceptfd, argv, msg.arglen);
			break;
		case cmd_unconnect:
			cmd_unconnect_func(acceptfd, argv, msg.arglen);
			break;
		case cmd_register:
			cmd_register_func(acceptfd, argv, msg.arglen);
			break;
		case cmd_unregister:
			cmd_unregister_func(acceptfd, argv, msg.arglen);
			break;
		case cmd_heart:
			cmd_heart_func(acceptfd, argv, msg.arglen);
			break;
		case cmd_max:
		default:
			break;
		}
	}
	close(acceptfd);
	return ;
}

void *thread_func(void *arg)
{
	int fd;
	while (1)
	{
		pthread_mutex_lock(&mutex);
		while (client_start >= client_end)
		{
			pthread_cond_wait(&cond, &mutex);
			continue;
		}

		fd = *client_start;
		*client_start = -1;
		client_start++;
		pthread_mutex_unlock(&mutex);
		if (fd > 0)
			handle_request(fd);
	}
}

int main(int argc, char ** argv)
{
	int i = 0;
	int sockfd = 0;
	int acceptfd = 0;
	socklen_t client_addr_len = 0;
	struct sockaddr_in server_addr, client_addr;

	char client_ip[16] = { 0 };

	pthread_t tids[THREAD_COUNT];

	client_start = client_end = clientfd;

	memset(&server_addr, 0, sizeof(server_addr));
	memset(&client_addr, 0, sizeof(client_addr));

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		handle_error("socket");
	}

	int val = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0)
	{
		handle_error("setsockopt()");
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(DIS_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		close(sockfd);
		handle_error("bind");
	}

	if (listen(sockfd, LISTEN_BACKLOG) < 0)
	{
		close(sockfd);
		handle_error("listen");
	}

	for (i = 0; i < QUEUE_MAX; ++i)
	{
		clientfd[i] = -1;
	}

	for (i = 0; i < QUEUE_MAX; ++i)
	{
		serverlist[i].client_count = 0;
		serverlist[i].weight = WEIGHT_TOP;
		serverlist[i].info.id = -1;
	}

	for (i = 0; i < THREAD_COUNT; ++i)
	{
		if (pthread_create(tids + i, NULL, thread_func, NULL) != 0)
		{
			close(sockfd);
			handle_error("pthread_create");
		}
	}

	while (1)
	{
		client_addr_len = sizeof(client_addr);
		if ((acceptfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0)
		{
			perror("accept");
			continue;
		}

		memset(client_ip, 0, sizeof(client_ip));
		inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
		printf("client:%s:%d\n", client_ip, ntohs(client_addr.sin_port));

		//	pthread_mutex_lock(&mutex);
		*client_end = acceptfd;
		client_end++;
		//	pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&cond);
	}

	close(sockfd);

	return 0;
}