#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>
#include <poll.h>

#include "public_head.h"
#include "fileio.h"

#define LISTEN_BACKLOG 50
#define MAX_CLIENT	1024

static ssize_t handle_request(int acceptfd)
{
	ssize_t readret = 0;
	char read_buff[256] = { 0 };
	char write_buff[256] = { 0 };


	memset(read_buff, 0, sizeof(read_buff));
	readret = read(acceptfd, read_buff, sizeof(read_buff));
	if (readret == 0)
		return readret;

	printf("acceptfd:%d, recv message:%s\n", acceptfd, read_buff);

	memset(write_buff, 0, sizeof(write_buff));
	sprintf(write_buff, "This is server send message");
	write(acceptfd, write_buff, sizeof(write_buff));

	printf("\n");
	return readret;
}

int main(int argc, char ** argv)
{
	int i = 0;
    int sockfd = 0;
    int acceptfd = 0;
    socklen_t client_addr_len = 0;
    struct sockaddr_in server_addr, client_addr;

    char client_ip[16] = { 0 };

	int ready;
	int clientlen = 0;
	struct pollfd clientfd[MAX_CLIENT];

	for (i = 0; i < MAX_CLIENT; ++i)
		clientfd[i].fd = -1;

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        handle_error("socket");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9527);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
		char buff[256] = { 0 };
        close(sockfd);
		strerror_r(errno, buff, sizeof(buff));
        handle_error("bind");
    }

    if(listen(sockfd, LISTEN_BACKLOG) < 0)
    {
        close(sockfd);
        handle_error("listen");
    }

	clientfd[0].fd = sockfd;
	clientfd[0].events = POLLRDNORM;
	clientlen++;
	
    while(1)
    {
		ready = poll(clientfd, clientlen, 0);
		if (ready < 0)
		{
			if(errno == EINTR)
				continue;

			close(sockfd);
			handle_error("poll");
		}
		else if (ready == 0)
		{
			continue;
		}

		if (clientfd[0].revents & POLLRDNORM)
		{
			client_addr_len = sizeof(client_addr);
			if ((acceptfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0)
			{
				handle_warning("accept");
				continue;
			}

			memset(client_ip, 0, sizeof(client_ip));
			inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
			printf("client:%s:%d\n", client_ip, ntohs(client_addr.sin_port));

			clientfd[clientlen].fd = acceptfd;
			clientfd[clientlen].events = POLLRDNORM;
			clientlen++;

			if (ready == 1)
				continue;
		}
        
		for (i = 1; i < clientlen; ++i)
		{
			if (clientfd[i].fd < 0)
				continue;

			if (clientfd[i].revents & POLLRDNORM)
			{
				if (handle_request(clientfd[i].fd) <= 0)
				{
					close(clientfd[i].fd);
					clientfd[i].fd = -1;
				}
			}
		}
    }
    
    close(sockfd);

	exit(EXIT_SUCCESS);
}
