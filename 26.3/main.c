#define _XOPEN_SOURCE 500
#include <sys/wait.h>
#include "print_wait_status.h"
#include "../lib/tlpi_hdr.h"

int main(int argc, char *argv[])
{
    siginfo_t infop;
    pid_t childPid;

    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        usageErr("%s [exit-status]\n", argv[0]);
    }

    switch (fork()) {
    case -1: errExit("fork");

    case 0:             /* Child: either exits immediately with given
                           status or loops waiting for signals */
        printf("Child started with PID = %ld\n", (long) getpid());
        if (argc > 1) {                   /* Status supplied on command line? */
            exit(getInt(argv[1], 0, "exit-status"));
        } else {                           /* Otherwise, wait for signals */
            for (;;) {
                pause();
            }
        }
        exit(EXIT_FAILURE);             /* Not reached, but good practice */

    default:            /* Parent: repeatedly wait on child until it
                           either exits or is terminated by a signal */
        for (;;) {
            memset(&infop, 0, sizeof(siginfo_t));
            childPid = waitid(P_ALL, 0, &infop, WEXITED | WUNTRACED
#ifdef WCONTINUED       /* Not present on older versions of Linux */
                                                | WCONTINUED
#endif
                    );
            if (childPid == -1) {
                errExit("waitid");
            }

            /* Print status in hex, and as separate decimal bytes */
            printf("waitid() returned: PID=%ld\n", (long) infop.si_pid);
            printWaitStatus(NULL, &infop);

            if (WIFEXITED(infop.si_status) || WIFSIGNALED(infop.si_status)) {
                exit(EXIT_SUCCESS);
            }
        }
    }

    return 0;
}