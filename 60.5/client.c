#define _GNU_SOURCE
#include <signal.h>
#include <termios.h>
#include <stdio.h>
#include <sys/select.h>

#include "../lib/socket/inet_sockets.h"
#include "../lib/tlpi_hdr.h"
#include "../lib/tty/tty.h"

#define BUF_SIZE 256

struct termios tty_orig;

static void tty_reset(void)
{
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty_orig) == -1) {
        perror("tcsetattr");
        exit(1);
    }
}

static void exit_func(void)
{
    tty_reset();
}

int main(int argc, char *argv[])
{
    int sfd;
    ssize_t num_read;
    fd_set in_fds;
    char buf[BUF_SIZE];
    
    if (argc != 3 || strcmp(argv[1], "--help") == 0) {
        usageErr("%s host port\n", argv[0]);
    }

    sfd = inetConnect(argv[1], argv[2], SOCK_STREAM);
    if (sfd == -1) {
        errExit("inetConnect");
    }

    printf("%s\n", "successfully connected");

    tty_set_raw(STDIN_FILENO, &tty_orig);

    if (atexit(exit_func) != 0) {
        perror("atexit");
        exit(1);
    }

    for (;;) {
        int sd;
        FD_ZERO(&in_fds);
        FD_SET(STDIN_FILENO, &in_fds);
        FD_SET(sfd, &in_fds);

        sd = select(sfd + 1, &in_fds, NULL, NULL, NULL);
        if (sd == -1) {
            perror("select");
            exit(1);
        }

        if (FD_ISSET(STDIN_FILENO, &in_fds)) {
            num_read = read(STDIN_FILENO, buf, BUF_SIZE);
            if (num_read <= 0) {
                exit(EXIT_SUCCESS);
            }

            if (write(sfd, buf, num_read) != num_read) {
                fprintf(stderr, "partial/failed write\n");
                exit(1);
            }
        }

        if (FD_ISSET(sfd, &in_fds)) {
            num_read = read(sfd, buf, BUF_SIZE);
            if (num_read <= 0) {
                exit(EXIT_SUCCESS);
            }

            if (write(STDOUT_FILENO, buf, num_read) != num_read) {
                fprintf(stderr, "partial/failed write\n");
                exit(1);
            }
        }
    }

    if (shutdown(sfd, SHUT_WR) == -1) {
        errExit("shutdown");
    }
    
    exit(EXIT_SUCCESS);
}