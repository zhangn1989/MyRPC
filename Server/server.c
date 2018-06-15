#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
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
    int acceptfd = 0;
    socklen_t client_addr_len = 0;
    struct sockaddr_in server_addr, client_addr;

    char buff[256] = {0};

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

    client_addr_len = sizeof(client_addr);
    if((acceptfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0)
    {
        close(sockfd);
        handle_error("accept");
    }
    
    while(1)
    {
#if 0
        memset(buff, 0, sizeof(buff));
    //if cleint close, read funtion return 0.
        if(read(acceptfd, buff, sizeof(buff)) == 0)
          break;
        printf("%s\n", buff);
        write(acceptfd, "world", sizeof("world"), 0);
#endif
        sleep(1);
    }

    close(acceptfd);
    close(sockfd);

    return 0;
}
