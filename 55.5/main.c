#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define PORT_NUM_A 50002
#define PORT_NUM_B 50003

int main(int argc, char *argv[])
{
    int sock_a_fd, sock_b_fd, sock_fd;
    struct sockaddr_in addr_a, addr_b;
    char *msg;
    pid_t pid;

    memset(&addr_a, 0, sizeof(struct sockaddr_in));
    addr_a.sin_family = AF_INET;
    addr_a.sin_addr.s_addr = INADDR_ANY;
    addr_a.sin_port = htons(PORT_NUM_A);

    sock_a_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_a_fd == -1) {
        perror("socket");
        exit(1);
    }

    if (bind(sock_a_fd, (struct sockaddr *)&addr_a, sizeof(struct sockaddr_in)) == -1) {
        perror("bind");
        exit(1);
    }

    memset(&addr_b, 0, sizeof(struct sockaddr_in));
    addr_b.sin_family = AF_INET;
    addr_b.sin_addr.s_addr = INADDR_ANY;
    addr_b.sin_port = htons(PORT_NUM_B);

    sock_b_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_b_fd == -1) {
        perror("socket");
        exit(1);
    }

    if (bind(sock_b_fd, (struct sockaddr *)&addr_b, sizeof(struct sockaddr_in)) == -1) {
        perror("bind");
        exit(1);
    }

    if (connect(sock_a_fd, (struct sockaddr *)&addr_b, sizeof(struct sockaddr_in)) == -1) {
        perror("connect");
        exit(1);
    }

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
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

        if (sendto(sock_fd, msg, strlen(msg), 0, (struct sockaddr *)&addr_a, sizeof(struct sockaddr_in)) == -1) {
            perror("sendto");
            exit(1);
        }
    }

    return 0;
}