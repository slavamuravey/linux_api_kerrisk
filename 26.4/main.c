#define _XOPEN_SOURCE 500
#include <signal.h>
#include <sys/types.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include "../lib/tlpi_hdr.h"
#define CMD_SIZE 200

volatile sig_atomic_t child_ready = 0;

void sigusr1_handler(int s)
{
    child_ready = 1;
}

int main(int argc, char *argv[])
{
    char cmd[CMD_SIZE];
    pid_t childPid;
    sigset_t block_set, empty_set;
    struct sigaction act;
    
    setbuf(stdout, NULL);
    printf("Parent PID=%ld\n", (long) getpid());

    memset(&act, 0, sizeof(struct sigaction));
    sigemptyset(&act.sa_mask);
    act.sa_handler = sigusr1_handler;
    sigaction(SIGUSR1, &act, NULL);

    sigemptyset(&block_set);
    sigaddset(&block_set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &block_set, NULL);

    sigemptyset(&empty_set);
    
    switch (childPid = fork()) {
    case -1:
        errExit("fork");
    case 0:
        printf("Child (PID=%ld) exiting\n", (long) getpid());
        kill(getppid(), SIGUSR1);
        _exit(EXIT_SUCCESS);
    default:
        while (!child_ready) {
            sigsuspend(&empty_set);
        }
        
        snprintf(cmd, CMD_SIZE, "ps | grep %s", basename(argv[0]));
        system(cmd);
        if (kill(childPid, SIGKILL) == -1) {
            errMsg("kill");
        }
        
        sleep(3);
        printf("After sending SIGKILL to zombie (PID=%ld):\n", (long) childPid);
        system(cmd);
        exit(EXIT_SUCCESS);
    }
}