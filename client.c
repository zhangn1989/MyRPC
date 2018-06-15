#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */

#define LISTEN_BACKLOG 50
#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char ** argv)
{
    int sockfd = 0;
    char buff[256] = { 0 };
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

    while(1)
    {
        write(sockfd, "hello ", sizeof("hello "));
    //    read(sockfd, buff, sizeof(buff));
    //    printf("%s\n", buff);
        usleep(100);
    }
#if 0
    while(1)
    {
        read(acceptfd, buff, sizeof(buff));
        write(acceptfd, "world", sizeof("world"));
        printf("%s", buff);
    }
#endif
    close(sockfd);

    return 0;
}
