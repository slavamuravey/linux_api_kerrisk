#define _POSIX_C_SOURCE 200112L
#include <semaphore.h>
#include <time.h>
#include "../lib/tlpi_hdr.h"

#define SEM_TIMEDRECEIVE_TIMEOUT_SEC 3

int main(int argc, char *argv[])
{
    sem_t *sem;
    struct timespec to;
    memset(&to, 0, sizeof(to));

    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        usageErr("%s sem-name\n", argv[0]);
    }

    sem = sem_open(argv[1], 0);
    if (sem == SEM_FAILED) {
        errExit("sem_open");
    }

    clock_gettime(CLOCK_REALTIME, &to);
    to.tv_sec += SEM_TIMEDRECEIVE_TIMEOUT_SEC;
        
    if (sem_timedwait(sem, &to) == -1) {
        errExit("sem_timedwait");
    }
        
    printf("%ld sem_wait() succeeded\n", (long) getpid());
    exit(EXIT_SUCCESS);
}