#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#define BACKLOG 5

int main(int argc, char *argv[])
{
    int sock_fd;
    struct sockaddr_in addr, addr_bind;
    socklen_t addr_len;
    char ip_str[INET_ADDRSTRLEN];

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        exit(1);
    }

    memset(&addr_bind, 0, sizeof(struct sockaddr_in));
    addr_bind.sin_family = AF_INET;
    addr_bind.sin_addr.s_addr = INADDR_ANY;
    addr_bind.sin_port = htons(50000);

/*     if (bind(sock_fd, (struct sockaddr *)&addr_bind, sizeof(struct sockaddr_in)) == -1) {
        perror("bind");
        exit(1);
    } */

    if (listen(sock_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    addr_len = sizeof(addr);
    if (getsockname(sock_fd, (struct sockaddr *)&addr, &addr_len) == -1) {
        perror("getsockname");
        exit(1);
    }

    inet_ntop(AF_INET, &(addr.sin_addr), ip_str, INET_ADDRSTRLEN);
    printf("family: %d, ip: %s, port: %d, len: %d\n", addr.sin_family, ip_str, ntohs(addr.sin_port), addr_len);

    return 0;
}