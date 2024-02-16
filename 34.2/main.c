#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

void handler(int s)
{
}

int main(int argc, char *argv[])
{
    pid_t pid;
    struct sigaction sa;
    sigset_t set, emptyset;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sigemptyset(&emptyset);
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigprocmask(SIG_SETMASK, &set, NULL);

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        char *cmd[3] = {"sleep", "1000", NULL};
        sigaction(SIGUSR1, &sa, NULL);
        sigsuspend(&emptyset);
        printf("pgid is set\n");
        
        execvp(*cmd, cmd);
        perror(*cmd);
        exit(1);
    }

    if (setpgid(pid, 0) == -1) {
        perror("setpgid");
        exit(1);
    }

    if (kill(pid, SIGUSR1) == -1) {
        perror("kill");
        exit(1);
    }

    sleep(1);

    if (setpgid(pid, getpgid(getpid())) == -1) {
        perror("setpgid");
        exit(1);
    }

    printf("never executed\n");

    if (wait(NULL) == -1) {
        perror("wait");
        exit(1);
    }

    return 0;
}