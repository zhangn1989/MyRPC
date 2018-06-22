#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>  
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>

#include "public_head.h"
#include "fileio.h"

#define LISTEN_BACKLOG 50
#define MAX_EVENTS	5

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

	int epfd = -1, ready = -1;
	struct epoll_event ev;
	struct epoll_event evlist[MAX_EVENTS];

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        handle_error("socket");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9527);
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
	
	epfd = epoll_create1(0);
	if (epfd < 0)
	{
		close(sockfd);
		handle_error("epoll_create1");
	}

	ev.data.fd = sockfd;
	ev.events = EPOLLIN;
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev) < 0)
	{
		close(sockfd);
		close(epfd);
		handle_error("epoll_ctl");
	}

    while(1)
    {
		ready = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
		if (ready < 0)
		{
			if (errno == EINTR)
				continue;

			close(sockfd);
			close(epfd);
			handle_error("epoll_wait");
		}
		else if (ready == 0)
		{
			continue;
		}

		for (i = 0; i < ready; ++i)
		{
			if (evlist[i].events != EPOLLIN)
				continue;

			if (evlist[i].data.fd == sockfd)
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

				ev.data.fd = acceptfd;
				ev.events = EPOLLIN;
				if (epoll_ctl(epfd, EPOLL_CTL_ADD, acceptfd, &ev) < 0)
				{
					close(acceptfd);
					handle_warning("epoll_ctl");
					continue;
				}
			}
			else
			{
				if (handle_request(evlist[i].data.fd) <= 0)
				{
					ev.data.fd = evlist[i].data.fd;
					ev.events = EPOLLIN;
					if (epoll_ctl(epfd, EPOLL_CTL_DEL, evlist[i].data.fd, &ev) < 0)
						handle_warning("epoll_ctl");
					close(evlist[i].data.fd);
				}
			}
		}
    }

	close(epfd);
    close(sockfd);

	exit(EXIT_SUCCESS);
}
