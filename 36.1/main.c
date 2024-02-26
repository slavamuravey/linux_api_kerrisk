#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>

void print_rusage(int who)
{
    struct rusage rusage;
    if (getrusage(who, &rusage) == -1) {
        perror("getrusage");
        exit(1);
    }

    printf("sys time + user time: %ld\n", rusage.ru_stime.tv_usec + rusage.ru_utime.tv_usec);
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
        exit(0);
    }

    print_rusage(RUSAGE_CHILDREN);

    if (wait(NULL) == -1) {
        perror("wait");
        exit(1);
    }

    print_rusage(RUSAGE_CHILDREN);

    return 0;
}