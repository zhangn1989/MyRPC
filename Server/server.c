#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <semaphore.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>

#include "public_head.h"
#include "fileio.h"

#define LISTEN_BACKLOG 50

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
    int sockfd = -1;
    int acceptfd = -1;
    socklen_t client_addr_len = 0;
    struct sockaddr_in server_addr, client_addr;

    char client_ip[16] = { 0 };

	int clientfd[FD_SETSIZE];
	int i = 0, client_index = 0;
	int ready = -1, nfds = -1;
	fd_set rset;
	struct timeval timeout = { 0, 0 };
	FD_ZERO(&rset);

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

	for (i = 0; i < FD_SETSIZE; ++i)
		clientfd[i] = -1;

	int fd_stdin = fileno(stdin);
	nfds = fd_stdin > sockfd ? fd_stdin : sockfd;
	
    while(1)
    {
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		FD_SET(sockfd, &rset);
		for (i = 0; i < client_index; ++i)
		{
			if( clientfd[i] < 0)
				continue;

			FD_SET(clientfd[i], &rset);
		}

		ready = select(nfds + 1, &rset, NULL, NULL, &timeout);
		if (ready < 0)
		{
			if (errno == EINTR)
				continue;

			close(sockfd);
			handle_error("select");
		}
		else if (ready == 0)
		{
			continue;
		}

		if (FD_ISSET(sockfd, &rset))
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

			clientfd[client_index++] = acceptfd;

			FD_SET(acceptfd, &rset);
			if (acceptfd > nfds)
				nfds = acceptfd;
			if (ready == 1)
				continue;
		}

		for (i = 0; i < client_index; ++i)
		{
			if (clientfd[i] < 0)
				continue;

			if (FD_ISSET(clientfd[i], &rset))
			{
				if (handle_request(clientfd[i]) <= 0)
				{
					FD_CLR(clientfd[i], &rset);
					close(clientfd[i]);
					clientfd[i] = -1;
				}
			}
		}

// 		for (i = 0; i < client_index; ++i)
// 		{
// 			if (FD_ISSET(clientfd[i], &writefds))
// 			{
// 
// 			}
// 		}
    }
    
    close(sockfd);

	exit(EXIT_SUCCESS);
}
