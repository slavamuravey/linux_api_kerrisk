#define _GNU_SOURCE
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "./us_seqnum.h"

#define BACKLOG 5

int main(int argc, char *argv[])
{
    int sock_fd;
    struct sockaddr_un srv_addr;
    struct request req;
    struct response resp;
    int seq_num;
    
    seq_num = 0;

    memset(&srv_addr, 0, sizeof(struct sockaddr_un));
    srv_addr.sun_family = AF_UNIX;
    strncpy(srv_addr.sun_path, SV_SOCK_PATH, sizeof(srv_addr.sun_path) - 1);

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        exit(1);
    }

    if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT) {
        perror(SV_SOCK_PATH);
        exit(1);
    }

    if (bind(sock_fd, (struct sockaddr *)&srv_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sock_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    for (;;) {
        int cl_fd;
        struct sockaddr_un peer_addr;
        socklen_t peer_addr_size = sizeof(peer_addr);
        cl_fd = accept(sock_fd, &peer_addr, &peer_addr_size);
        if (cl_fd == -1) {
            perror("accept");
            continue;
        }

        printf("%s accepted\n", peer_addr.sun_path);

        if (read(cl_fd, &req, sizeof(struct request)) != sizeof(struct request)) {
            fprintf(stderr, "error reading request; discarding\n");
            continue;
        }

        resp.seq_num = seq_num;
        if (write(cl_fd, &resp, sizeof(struct response)) != sizeof(struct response)) {
            fprintf(stderr, "error writing to socket %s\n", peer_addr.sun_path);
        }
            
        if (close(cl_fd) == -1) {
            perror("close");
        }

        seq_num += req.seq_len;
    }
}
