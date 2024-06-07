#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    int fd;
    char buf[1];
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        fd = open("/dev/tty", O_RDONLY | O_NONBLOCK);
        if (fd == -1) {
            perror("open");
            exit(1);
        }

        sleep(1);

        printf("child: SID=%ld PID=%ld PGID=%ld PPID=%ld\n", (long)getsid(getpid()), (long)getpid(), (long)getpgrp(), (long)getppid());
        
        read(fd, buf, sizeof(buf));
        perror("read");

        exit(0);
    }

    printf("parent: SID=%ld PID=%ld PGID=%ld PPID=%ld\n", (long)getsid(getpid()), (long)getpid(), (long)getpgrp(), (long)getppid());

    return 0;
}
