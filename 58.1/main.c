#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

int my_isatty(int fd)
{
    struct termios t;
    
    return tcgetattr(fd, &t) == -1 ? 0 : 1;
}

void test_isatty(int fd)
{
    if (my_isatty(fd)) {
        printf("tty\n");
    } else {
        perror("isatty");
    }
}

int main(int argc, char *argv[])
{
    int fd;
    fd = open("/dev/null", O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    
    test_isatty(STDIN_FILENO);
    test_isatty(fd);
    test_isatty(100500);

    close(fd);

    return 0;
}