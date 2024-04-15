#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

int my_pipe(int pipefd[2])
{
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pipefd) == -1) {
        return -1;
    }

    if (shutdown(pipefd[0], SHUT_WR) == -1) {
        return -1;
    }

    if (shutdown(pipefd[1], SHUT_RD) == -1) {
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int fds[2];
    char buf[1];
    int counter;
    ssize_t nbytes;
    int flags;
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    if (my_pipe(fds) == -1) {
        perror("pipe");
        exit(1);
    }

    flags = fcntl(fds[1], F_GETFL, 0);
    fcntl(fds[1], F_SETFL, flags | O_NONBLOCK); 

    buf[0] = '.';
    counter = 0;

    while (1) {
        nbytes = sizeof(buf);
        if (write(fds[1], ".", nbytes) < nbytes) {
            if (errno == EWOULDBLOCK) {
                printf("bytes in buffer: %d\n", counter);
                break;
            }
            perror("write");
            exit(1);
        }

        counter++;
    }

    nbytes = read(fds[0], buf, sizeof(buf));
    if (nbytes == -1) {
        if (errno == EINTR) {
            printf("reading interrupted");
            exit(0);
        }
        
        perror("read");
        exit(1);
    }

    if (write(fds[0], ".", 1) == -1) {
        perror("write");
        exit(0);
    }

    return 0;
}
