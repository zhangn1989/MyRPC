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

#define LISTEN_BACKLOG 50
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char ** argv)
{
    int i = 0;
    int sockfd = 0;
    ssize_t readret = 0;
    char read_buff[256] = { 0 };
    char write_buff[256] = { 0 };
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        handle_error("socket");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9527);
    inet_pton(sockfd, "127.0.0.1", &server_addr.sin_addr.s_addr);

    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        close(sockfd);
        handle_error("connect");
    }

    for(i = 0; i < 10; ++i)
    {
        memset(write_buff, 0, sizeof(write_buff));
        sprintf(write_buff, "This is client send message:%d", i);
        write(sockfd, write_buff, strlen(write_buff) + 1);

        memset(read_buff, 0, sizeof(read_buff));
        readret = read(sockfd, read_buff, sizeof(read_buff));
        if(readret == 0)
            break;
        printf("%s\n", read_buff);      
        
        sleep(1);
    }

    close(sockfd);

    return 0;
}
