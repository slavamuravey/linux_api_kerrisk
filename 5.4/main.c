#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

static char *filename1 = "file1.txt";
static char *filename2 = "file2.txt";

void open_files2(int fds[2])
{
    int fd1, fd2;
    
    fd1 = open(filename1, O_CREAT | O_RDWR, 0666);
    if (fd1 == -1) {
        perror(filename1);
        exit(EXIT_FAILURE);
    }

    fd2 = open(filename2, O_CREAT | O_RDWR, 0666);
    if (fd2 == -1) {
        perror(filename2);
        exit(EXIT_FAILURE);
    }

    fds[0] = fd1;
    fds[1] = fd2;
}

int my_dup(int fd)
{
    return fcntl(fd, F_DUPFD, fd);
}

int my_dup2(int fd, int new_fd)
{
    if (fcntl(fd, F_GETFL) == -1) {
        errno = EBADF;
        
        return -1;
    }

    if (fd == new_fd) {
        return new_fd;
    }
    
    close(new_fd);
    
    return fcntl(fd, F_DUPFD, new_fd);
}

int main()
{
    int fds[2];
    int fd1, fd2, new_fd;
    char *buf;
    
    open_files2(fds);
    fd1 = fds[0];
    fd2 = fds[1];

    new_fd = dup(fd1);

    printf("dup: old fd: %d, new fd: %d\n", fd1, new_fd);

    new_fd = dup(777);
    if (new_fd == -1) {
        perror("dup");
    }

    new_fd = dup2(fd1, 7);

    printf("dup2: old fd: %d, new fd: %d\n", fd1, new_fd);

    new_fd = dup2(777, 8);
    if (new_fd == -1) {
        perror("dup2");
    }

    new_fd = dup2(fd1, fd2);
    if (new_fd == -1) {
        perror("dup2");
        exit(EXIT_FAILURE);
    }
    
    buf = "dup test";

    if (write(fd2, buf, strlen(buf)) == -1) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    open_files2(fds);
    fd1 = fds[0];
    fd2 = fds[1];

    /* my_dup* functions usage */

    new_fd = my_dup(fd1);

    printf("my_dup: old fd: %d, new fd: %d\n", fd1, new_fd);

    new_fd = my_dup(777);
    if (new_fd == -1) {
        perror("my_dup");
    }

    new_fd = my_dup2(fd1, 9);

    printf("my_dup2: old fd: %d, new fd: %d\n", fd1, new_fd);

    new_fd = my_dup2(777, 8);
    if (new_fd == -1) {
        perror("my_dup2");
    }

    new_fd = my_dup2(fd1, fd2);
    if (new_fd == -1) {
        perror("my_dup2");
        exit(EXIT_FAILURE);
    }
    
    buf = "my_dup test";

    if (write(fd2, buf, strlen(buf)) == -1) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    return 0;
}