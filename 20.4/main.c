#define _GNU_SOURCE
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int my_siginterrupt(int sig, int flag)
{
    struct sigaction sa;
    if (sigaction(sig, NULL, &sa) == -1) {
        return -1;
    }

    if (flag) {
        sa.sa_flags &= ~SA_RESTART;
    } else {
        sa.sa_flags |= SA_RESTART;
    }

    return sigaction(sig, &sa, NULL);
}

void handler(int s)
{
    printf("sigint\n");
}

int main(int argc, char *argv[])
{
    char buf[3];
    ssize_t bytes;
    signal(SIGINT, handler);
    my_siginterrupt(SIGINT, 1);
    
    bytes = read(STDIN_FILENO, buf, sizeof(buf));
    if (bytes == -1) {
        perror("read");
    }

    my_siginterrupt(SIGINT, 0);

    bytes = read(STDIN_FILENO, buf, sizeof(buf));
    if (bytes == -1) {
        perror("read");
    }

    return 0;
}