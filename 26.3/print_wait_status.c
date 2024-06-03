#define _GNU_SOURCE     /* Get strsignal() declaration from <string.h> */
#include <string.h>
#include <sys/wait.h>
#include "print_wait_status.h"  /* Declaration of printWaitStatus() */
#include "../lib/tlpi_hdr.h"

void printWaitStatus(const char *msg, siginfo_t *infop)
{
    if (msg != NULL) {
        printf("%s", msg);
    }

    if (infop == NULL) {
        return;
    }

    printf("si_code: ");
    switch (infop->si_code) {
    case CLD_EXITED:
        printf("CLD_EXITED\n");
        break;
    case CLD_KILLED:
        printf("CLD_KILLED\n");
        break;
    case CLD_STOPPED:
        printf("CLD_STOPPED\n");
        break;
    case CLD_CONTINUED:
        printf("CLD_CONTINUED\n");
        break;
    case CLD_DUMPED:
        printf("CLD_DUMPED\n");
        break;
    case CLD_TRAPPED:
        printf("CLD_TRAPPED\n");
        break;
    default:
        printf("unknown\n");
    }

    printf("si_pid: %d\n", infop->si_pid);
    printf("si_signo: %d\n", infop->si_signo);
    printf("si_status: %d\n", infop->si_status);
    printf("si_uid: %d\n", infop->si_uid);
}