#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define SOCKET_PATH_A "/tmp/socket_a"
#define SOCKET_PATH_B "/tmp/socket_b"

#define BUF_SIZE 10

int main(int argc, char *argv[])
{
    int sock_a_fd, sock_b_fd, sock_fd;
    struct sockaddr_un addr_a, addr_b;
    char *msg;
    pid_t pid;

    memset(&addr_a, 0, sizeof(struct sockaddr_un));
    addr_a.sun_family = AF_UNIX;
    strncpy(addr_a.sun_path, SOCKET_PATH_A, sizeof(addr_a.sun_path) - 1);

    sock_a_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_a_fd == -1) {
        perror("socket");
        exit(1);
    }

    if (remove(SOCKET_PATH_A) == -1 && errno != ENOENT) {
        perror(SOCKET_PATH_A);
        exit(1);
    }

    if (bind(sock_a_fd, (struct sockaddr *)&addr_a, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        exit(1);
    }

    memset(&addr_b, 0, sizeof(struct sockaddr_un));
    addr_b.sun_family = AF_UNIX;
    strncpy(addr_b.sun_path, SOCKET_PATH_B, sizeof(addr_b.sun_path) - 1);

    sock_b_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_b_fd == -1) {
        perror("socket");
        exit(1);
    }

    if (remove(SOCKET_PATH_B) == -1 && errno != ENOENT) {
        perror(SOCKET_PATH_B);
        exit(1);
    }

    if (bind(sock_b_fd, (struct sockaddr *)&addr_b, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        exit(1);
    }

    if (connect(sock_a_fd, (struct sockaddr *)&addr_b, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        exit(1);
    }

    sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        exit(1);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        msg = "some message";

        if (sendto(sock_fd, msg, strlen(msg), 0, (struct sockaddr *)&addr_a, sizeof(struct sockaddr_un)) == -1) {
            perror("sendto");
            exit(1);
        }
    }

    return 0;
}