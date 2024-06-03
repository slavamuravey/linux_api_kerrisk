#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    pid_t parent_pid, child_pid;
    parent_pid = fork();
    if (parent_pid == -1) {
        perror("fork");
        exit(1);
    }

    if (parent_pid == 0) {
        child_pid = fork();
        if (child_pid == -1) {
            perror("fork");
            exit(1);
        }
        
        if (child_pid == 0) {
            sleep(1);
            /* A zombie process cannot be the parent of any process */
            printf("ppid: %d\n", getppid());
            exit(0);            
        }

        exit(0);
    }

    sleep(2);

    return 0;
}