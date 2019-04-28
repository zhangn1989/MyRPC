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

#define LISTEN_BACKLOG 50
#define THREAD_COUNT	3

static int clientfd[QUEUE_MAX];
static int *client_start;
static int *client_end;

static serverinfo selfinfo;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static void handle_request(int acceptfd)
{
	int i = 0; 
    ssize_t readret = 0;
    char read_buff[256] = { 0 };
    char write_buff[256] = { 0 };
   
	while (1)
	{
		memset(read_buff, 0, sizeof(read_buff));
		readret = read(acceptfd, read_buff, sizeof(read_buff));
		if (readret == 0)
			break;

		printf("thread id:%lu, recv message:%s\n", pthread_self(), read_buff);

		memset(write_buff, 0, sizeof(write_buff));
		sprintf(write_buff, "This is server send message:%d", i++);
		write(acceptfd, write_buff, sizeof(write_buff));
	}

    printf("\n");
    close(acceptfd);
    return;
}

static void *thread_func(void *arg)
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
		if(fd > 0)
			handle_request(fd);
	}
	return NULL;
}

static void register_service(in_port_t port)
{
	int sockfd = 0;
	struct sockaddr_in server_addr;

	message *sendmsg, *recvmsg;

	sendmsg = malloc(sizeof(message) + sizeof(serverinfo));
	if(!sendmsg)
		handle_error("register_service");

	recvmsg = malloc(sizeof(message) + sizeof(int));
	if (!recvmsg)
	{
		free(sendmsg);
		handle_error("register_service");
	}

	sendmsg->cmd = cmd_register;
	sendmsg->arglen = sizeof(serverinfo);
	selfinfo.port = port;
	get_local_ip(IF_NAME, selfinfo.ip);
	memcpy(sendmsg->argv, &selfinfo, sizeof(serverinfo));

	memset(&server_addr, 0, sizeof(server_addr));
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		free(sendmsg);
		free(recvmsg);
		handle_error("socket");
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(DIS_PORT);
	inet_pton(sockfd, DIS_IP, &server_addr.sin_addr.s_addr);

	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		free(sendmsg);
		free(recvmsg);
		close(sockfd);
		handle_error("connect");
	}

	writen(sockfd, sendmsg, sizeof(message) + sizeof(serverinfo));
	if(readn(sockfd, recvmsg, sizeof(message) + sizeof(int)) != 0)
		memcpy(&selfinfo.id, recvmsg->argv, sizeof(int));
	close(sockfd);

	free(sendmsg);
	free(recvmsg);
}

static void unregister_service()
{
	int sockfd = 0;
	struct sockaddr_in server_addr;

	message *sendmsg;

	sendmsg = malloc(sizeof(message) + sizeof(int));
	if (!sendmsg)
		handle_error("register_service");

	sendmsg->cmd = cmd_unregister;
	sendmsg->arglen = sizeof(int);
	memcpy(sendmsg->argv, &selfinfo.id, sendmsg->arglen);

	memset(&server_addr, 0, sizeof(server_addr));
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		free(sendmsg);
		handle_error("socket");
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(DIS_PORT);
	inet_pton(sockfd, DIS_IP, &server_addr.sin_addr.s_addr);

	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		free(sendmsg);
		close(sockfd);
		handle_error("connect");
	}

	writen(sockfd, sendmsg, sizeof(message) + sizeof(int));
	
	close(sockfd);
	free(sendmsg);
}

int main(int argc, char ** argv)
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

	if (argc < 2)
		handle_error("argc");

	port = atoi(argv[1]);

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		unregister_service();
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
    if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        close(sockfd);
		handle_error("bind");
    }

    if(listen(sockfd, LISTEN_BACKLOG) < 0)
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

	register_service(port);
	if (selfinfo.id == -1)
	{
		close(sockfd);
		handle_error("register_service");
	}

    while(1)
    {
        client_addr_len = sizeof(client_addr);
        if((acceptfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0)
        {
            perror("accept");
            continue;
        }
       
        memset(client_ip, 0, sizeof(client_ip));
        inet_ntop(AF_INET,&client_addr.sin_addr,client_ip,sizeof(client_ip)); 
        printf("client:%s:%d\n",client_ip,ntohs(client_addr.sin_port));

	//	pthread_mutex_lock(&mutex);
		*client_end = acceptfd;
		client_end++;
	//	pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&cond);
    }
    
    close(sockfd);

	unregister_service();
    return 0;
}
