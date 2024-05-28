#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid != 0) {
        exit(0);
    }

    sleep(1);

    printf("ppid = %d\n", getppid());

    return 0;
}