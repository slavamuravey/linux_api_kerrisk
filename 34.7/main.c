#define _XOPEN_SOURCE 500
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

void handler(int sig)
{
    printf("PID %ld: caught signal %2d (%s)\n", (long) getpid(), sig, strsignal(sig));
}

int main(int argc, char *argv[])
{
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        struct sigaction sa;

        if (argc > 1) {
            sa.sa_handler = handler;
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = 0;
            if (sigaction(SIGTTIN, &sa, NULL) == -1) {
                perror("sigaction");
                exit(1);
            }
            if (sigaction(SIGTTOU, &sa, NULL) == -1) {
                perror("sigaction");
                exit(1);
            }
            if (sigaction(SIGTSTP, &sa, NULL) == -1) {
                perror("sigaction");
                exit(1);
            }
        } else {
            signal(SIGTTIN, SIG_DFL);
            signal(SIGTTOU, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);
        }
        
        sleep(1);

        printf("child: SID=%ld PID=%ld PGID=%ld PPID=%ld\n", (long)getsid(getpid()), (long)getpid(), (long)getpgrp(), (long)getppid());
        
        pause();

        exit(0);
    }

    printf("parent: SID=%ld PID=%ld PGID=%ld PPID=%ld\n", (long)getsid(getpid()), (long)getpid(), (long)getpgrp(), (long)getppid());

    return 0;
}
