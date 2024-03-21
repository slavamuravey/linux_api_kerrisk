#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "./common.h"

int main(int argc, char *argv[])
{
    int fd;
    struct sockaddr_un srv_addr, cl_addr;
    ssize_t nbytes;
    socklen_t len;
    char buf[BUF_SIZE];

    memset(&srv_addr, 0, sizeof(struct sockaddr_un));
    srv_addr.sun_family = AF_UNIX;
    strncpy(srv_addr.sun_path, SV_SOCK_PATH, sizeof(srv_addr.sun_path) - 1);

    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("socket");
        exit(1);
    }

    if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT) {
        perror(SV_SOCK_PATH);
        exit(1);
    }

    if (bind(fd, (struct sockaddr *)&srv_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("socket");
        exit(1);
    }

    while (1) {
        printf("waiting for datagrams...\n");
        len = sizeof(struct sockaddr_un);
        
        nbytes = recvfrom(fd, buf, BUF_SIZE, 0, (struct sockaddr *)&cl_addr, &len);
        if (nbytes == -1) {
            perror("recvfrom");
            exit(1);
        }

        printf("server received %ld bytes from %s, payload: %s\n", (long)nbytes, cl_addr.sun_path, buf);

        printf("handling the datagram...\n");
        
        sleep(3);
    }

    return 0;
}
