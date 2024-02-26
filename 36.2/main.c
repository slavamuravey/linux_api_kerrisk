#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    struct rusage rusage;
    char **argv_prog;
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        argv_prog = argv + 1;
        execvp(*argv_prog, argv_prog);
        perror(*argv_prog);
        exit(1);
    }

    if (wait(NULL) == -1) {
        perror("wait");
        exit(1);
    }

    
    if (getrusage(RUSAGE_CHILDREN, &rusage) == -1) {
        perror("getrusage");
        exit(1);
    }

    printf("sys time: %ld\n", rusage.ru_stime.tv_usec);
    printf("user time: %ld\n", rusage.ru_utime.tv_usec);
    printf("total time: %ld\n", rusage.ru_stime.tv_usec + rusage.ru_utime.tv_usec);

    return 0;
}