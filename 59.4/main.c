#define _GNU_SOURCE
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <poll.h>
#include "../lib/tlpi_hdr.h"

static int pfd[2];

static void handler(int sig)
{
    int savedErrno;

    savedErrno = errno;
    if (write(pfd[1], "x", 1) == -1 && errno != EAGAIN) {
        errExit("write");
    }
        
    errno = savedErrno;
}

int main(int argc, char *argv[])
{
    int ready, nfds, flags;
    int pto;
    struct sigaction sa;
    char ch;
    int fd, j;
    struct pollfd *fds;

    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        usageErr("%s {timeout|-} fd...\n\t\t('-' means infinite timeout)\n", argv[0]);
    }

    if (strcmp(argv[1], "-") == 0) {
        pto = -1;
    } else {
        pto = getLong(argv[1], 0, "timeout") * 1000;
    }

    nfds = argc - 1;
    
    fds = calloc(nfds, sizeof(struct pollfd));
    if (fds == NULL) {
        errExit("calloc");
    }

    for (j = 0; j < nfds - 1; j++) {
        fd = getInt(argv[j + 2], 0, "fd");
        fds[j].fd = fd;
        fds[j].events = POLLIN;
    }

    if (pipe(pfd) == -1) {
        errExit("pipe");
    }

    fds[j].fd = pfd[0];
    fds[j].events = POLLIN;

    flags = fcntl(pfd[0], F_GETFL);
    if (flags == -1) {
         errExit("fcntl-F_GETFL");
    }
       
    flags |= O_NONBLOCK;
    if (fcntl(pfd[0], F_SETFL, flags) == -1) {
        errExit("fcntl-F_SETFL");
    }

    flags = fcntl(pfd[1], F_GETFL);
    if (flags == -1) {
        errExit("fcntl-F_GETFL");
    }
        
    flags |= O_NONBLOCK;
    if (fcntl(pfd[1], F_SETFL, flags) == -1) {
        errExit("fcntl-F_SETFL");
    }

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        errExit("sigaction");
    }

    while ((ready = poll(fds, nfds, pto)) == -1 && errno == EINTR) {
        continue;
    }

    if (ready == -1) {
        errExit("poll");
    }

    if (fds[j].revents & POLLIN) {
        printf("A signal was caught\n");

        for (;;) {
            if (read(pfd[0], &ch, 1) == -1) {
                if (errno == EAGAIN) {
                    break;
                } else {
                    errExit("read"); 
                }
            }
        }
    }

    printf("ready = %d\n", ready);
    
    for (j = 0; j < nfds - 1; j++) {
        fd = getInt(argv[j + 2], 0, "fd");
        printf("%d: %s\n", fd, fds[j].revents & POLLIN ? "r" : "");
    }

    printf("%d: %s   (read end of pipe)\n", pfd[0], fds[j].revents & POLLIN ? "r" : "");

    exit(EXIT_SUCCESS);
}