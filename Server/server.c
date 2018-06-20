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

static void handle_request(int acceptfd)
{
    int i = 0;
    ssize_t readret = 0;
    char read_buff[256] = { 0 };
    char write_buff[256] = { 0 };
   
    for(i = 0; i < 10; ++i)
    {
        memset(read_buff, 0, sizeof(read_buff));
        readret = read(acceptfd, read_buff, sizeof(read_buff));
        if(readret == 0)
            break;

        printf("progress id:%d, recv message:%s\n", getpid(), read_buff);

        memset(write_buff, 0, sizeof(write_buff));
        sprintf(write_buff, "This is server send message:%d", i);
        write(acceptfd, write_buff, sizeof(write_buff));
    }
    printf("\n");
    close(acceptfd);
    return;
}

int main(int argc, char ** argv)
{
    int sockfd = 0;
    int acceptfd = 0;
    socklen_t client_addr_len = 0;
    struct sockaddr_in server_addr, client_addr;

    char client_ip[16] = { 0 };

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
	
    while(1)
    {
        client_addr_len = sizeof(client_addr);
        if((acceptfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0)
        {
			handle_warning("accept");
			continue;
        }
       
        memset(client_ip, 0, sizeof(client_ip));
        inet_ntop(AF_INET,&client_addr.sin_addr,client_ip,sizeof(client_ip)); 
        printf("client:%s:%d\n",client_ip,ntohs(client_addr.sin_port));


    }
    
    close(sockfd);

	exit(EXIT_SUCCESS);
}
