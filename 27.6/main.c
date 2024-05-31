#define _GNU_SOURCE
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void handler(int sig)
{
    printf("signal handler\n");
}

int main(int argc, char *argv[])
{
    pid_t pid;
    struct sigaction sa;
    sigset_t blocked_mask;
    
    sa.sa_handler = handler;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    
    sigemptyset(&blocked_mask);
    sigaddset(&blocked_mask, SIGCHLD);
    
    if (sigprocmask(SIG_SETMASK, &blocked_mask, 0) == -1) {
        perror("sigprocmask");
        exit(1);
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        exit(0);
    }

    if (wait(NULL) == -1) {
        perror("wait");
        exit(1);
    }

    if (sigprocmask(SIG_UNBLOCK, &blocked_mask, 0) == -1) {
        perror("sigprocmask");
        exit(1);
    }

    return 0;
}