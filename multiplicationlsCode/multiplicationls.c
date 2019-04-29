#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>

#include "public_head.h"

#define LISTEN_BACKLOG	50
#define THREAD_COUNT	3
#define LISTEN_PORT		10010
#define ADD_IP		"127.0.0.1"
#define ADD_PORT	10086

static int clientfd[QUEUE_MAX];
static int* client_start;
static int* client_end;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static void handle_request(int acceptfd)
{
	ssize_t readret = 0;
	Message message;

	int i = 0;
	int sockfd = 0;
	Message addMessage;
	struct sockaddr_in addServer_addr;

	while (1)
	{
		memset(&message, 0, sizeof(message));
		readret = read(acceptfd, &message, sizeof(message));
		if (readret == 0)
			break;

		printf("thread id:%lu, recv operation:%d x %d\n",
			pthread_self(), message.arg1, message.arg2);

		memset(&addServer_addr, 0, sizeof(addServer_addr));
		memset(&addMessage, 0, sizeof(Message));

		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			handle_error("socket");

		addServer_addr.sin_family = AF_INET;
		addServer_addr.sin_port = htons(ADD_PORT);
		inet_pton(sockfd, ADD_IP, &addServer_addr.sin_addr.s_addr);

		if (connect(sockfd, (struct sockaddr*) & addServer_addr, sizeof(addServer_addr)) < 0)
		{
			close(sockfd);
			handle_error("connect");
		}

		for(i = 0; i < message.arg2; ++i)
		{
			addMessage.arg1 = message.arg1;
			addMessage.arg2 = addMessage.result;
			write(sockfd, &addMessage, sizeof(Message));
			readret = read(sockfd, &addMessage, sizeof(Message));
			sleep(1);
		}
		close(sockfd);
		message.result = addMessage.result;

		write(acceptfd, &message, sizeof(message));
	}

	printf("\n");
	close(acceptfd);
	return;
}

static void* thread_func(void* arg)
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
	return NULL;
}

int main(int argc, char** argv)
{
	int i = 0;
	int sockfd = 0;
	int acceptfd = 0;
	in_port_t port = 0;
	socklen_t client_addr_len = 0;
	struct sockaddr_in server_addr, client_addr;

	char client_ip[16] = { 0 };

	pthread_t tids[THREAD_COUNT];

	client_start = client_end = clientfd;

	port = LISTEN_PORT;

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
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sockfd, (struct sockaddr*) & server_addr, sizeof(server_addr)) < 0)
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
		if ((acceptfd = accept(sockfd, (struct sockaddr*) & client_addr, &client_addr_len)) < 0)
		{
			perror("accept");
			continue;
		}

		memset(client_ip, 0, sizeof(client_ip));
		inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
		printf("client:%s:%d\n", client_ip, ntohs(client_addr.sin_port));

		*client_end = acceptfd;
		client_end++;
		pthread_cond_signal(&cond);
	}

	close(sockfd);

	return 0;
}
