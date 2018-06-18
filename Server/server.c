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

#define LISTEN_BACKLOG 50
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

void *thread_func(void *arg)
{
    int i = 0;
    int acceptfd = *(int *)arg;

    ssize_t readret = 0;
    char read_buff[256] = { 0 };
    char write_buff[256] = { 0 };
   
    for(i = 0; i < 10; ++i)
    {
        memset(read_buff, 0, sizeof(read_buff));
        readret = read(acceptfd, read_buff, sizeof(read_buff));
        if(readret == 0)
            break;

        printf("thread id:%lu, recv message:%s\n", pthread_self(), read_buff);

        memset(write_buff, 0, sizeof(write_buff));
        sprintf(write_buff, "This is server send message:%d", i);
        write(acceptfd, write_buff, sizeof(write_buff));
    }
    printf("\n");
    close(acceptfd);
    return NULL;
}

int main(int argc, char ** argv)
{
    int sockfd = 0;
    int acceptfd = 0;
    socklen_t client_addr_len = 0;
    struct sockaddr_in server_addr, client_addr;

    char client_ip[16] = { 0 };

    pthread_t tid;

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


    while(1)
    {
        client_addr_len = sizeof(client_addr);
        if((acceptfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0)
        {
            close(sockfd);
            handle_error("accept");
        }
       
        memset(client_ip, 0, sizeof(client_ip));
        inet_ntop(AF_INET,&client_addr.sin_addr,client_ip,sizeof(client_ip)); 
        printf("client:%s:%d\n",client_ip,ntohs(client_addr.sin_port));

        if(pthread_create(&tid, NULL, thread_func, &acceptfd) != 0)
        {
            handle_error("pthread_create");
            break;
        }

        sleep(1);
    }

    close(sockfd);

    return 0;
}
