#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include "./common.h"

#define DGRAM_COUNT 1000

int main(int argc, char *argv[])
{
    int fd;
    struct sockaddr_un srv_addr, cl_addr;
    char msg[128];
    int i;

    fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("socket");
        exit(1);
    }

    memset(&cl_addr, 0, sizeof(struct sockaddr_un));
    cl_addr.sun_family = AF_UNIX;
    snprintf(cl_addr.sun_path, sizeof(cl_addr.sun_path), CL_SOCK_PATH_TPL, (long)getpid());

    if (bind(fd, (struct sockaddr *)&cl_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        exit(1);
    }

    memset(&srv_addr, 0, sizeof(struct sockaddr_un));
    srv_addr.sun_family = AF_UNIX;
    strncpy(srv_addr.sun_path, SV_SOCK_PATH, sizeof(srv_addr.sun_path) - 1);

    for (i = 0; i < DGRAM_COUNT; i++) {
        sprintf(msg, "[%d]", i);
        if (sendto(fd, msg, strlen(msg), 0, (struct sockaddr *)&srv_addr, sizeof(struct sockaddr_un)) == -1) {
            perror("sendto");
            exit(1);
        }
    }

    return 0;
}
