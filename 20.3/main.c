#define _GNU_SOURCE
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

void sigint_handler(int s)
{
    printf("sigint handler start\n");
    sleep(5);
    printf("sigint handler stop\n");
}

void sigquit_handler(int s)
{
    printf("sigquit handler invoked\n");
}

int main(int argc, char *argv[])
{
    struct sigaction sa_sigint;
    struct sigaction sa_sigquit;
    memset(&sa_sigint, 0, sizeof(sa_sigint));
    sigemptyset(&sa_sigint.sa_mask);
    sa_sigint.sa_handler = sigint_handler;
    sa_sigint.sa_flags = SA_NODEFER;
    sigaction(SIGINT, &sa_sigint, NULL);

    memset(&sa_sigquit, 0, sizeof(sa_sigquit));
    sigemptyset(&sa_sigquit.sa_mask);
    sa_sigquit.sa_handler = sigquit_handler;
    sa_sigquit.sa_flags = SA_RESETHAND;
    sigaction(SIGQUIT, &sa_sigquit, NULL);

    while (1) {
        pause();
    }

    return 0;
}