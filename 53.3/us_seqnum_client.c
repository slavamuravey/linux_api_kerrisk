#define _GNU_SOURCE
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include "./us_seqnum.h"
#include "../lib/tlpi_hdr.h"

static char client_sock[CL_SOCK_PATH_LEN];

static void remove_sock(void)
{
    unlink(client_sock);
}

int main(int argc, char *argv[])
{
    int sock_fd;
    struct request req;
    struct response resp;
    struct sockaddr_un srv_addr, cl_addr;

    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usageErr("%s [seq-len]\n", argv[0]);
    }

    if (atexit(remove_sock) != 0) {
        fprintf(stderr, "atexit error.\n");
        exit(EXIT_FAILURE);
    }

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        exit(1);
    }

    memset(&cl_addr, 0, sizeof(struct sockaddr_un));
    cl_addr.sun_family = AF_UNIX;
    snprintf(cl_addr.sun_path, sizeof(cl_addr.sun_path), CL_SOCK_PATH_TPL, (long)getpid());

    if (bind(sock_fd, (struct sockaddr *)&cl_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        exit(1);
    }

    memset(&srv_addr, 0, sizeof(struct sockaddr_un));
    srv_addr.sun_family = AF_UNIX;
    strncpy(srv_addr.sun_path, SV_SOCK_PATH, sizeof(srv_addr.sun_path) - 1);

    if (connect(sock_fd, (struct sockaddr *)&srv_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        exit(1);
    }

    req.pid = getpid();
    req.seq_len = (argc > 1) ? getInt(argv[1], GN_GT_0, "seq-len") : 1;

    if (write(sock_fd, &req, sizeof(struct request)) != sizeof(struct request)) {
        fprintf(stderr, "error writing to socket %s\n", SV_SOCK_PATH);
        exit(1);
    }
        
    if (read(sock_fd, &resp, sizeof(struct response)) != sizeof(struct response)) {
        fprintf(stderr, "can't read response from server\n");
        exit(1);
    }
        
    printf("%d\n", resp.seq_num);

    return 0;
}
