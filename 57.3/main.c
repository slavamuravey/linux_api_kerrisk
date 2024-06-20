#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <alloca.h>

#define BUFSIZE 4

ssize_t my_sendfile(int out_fd, int in_fd, off_t *offset, size_t count)
{
    char *buf;
    ssize_t nbytes, actual_bytes, want_bytes;
    off_t initial_offset;

    buf = alloca(count);
    
    if (offset != NULL) {
        initial_offset = lseek(in_fd, 0, SEEK_CUR);
        if (initial_offset == -1) {
            return -1;
        }
        
        if (lseek(in_fd, *offset, SEEK_SET) == -1) {
            return -1;
        }
    }
    
    nbytes = read(in_fd, buf, count);
    if (nbytes == -1) {
        return -1;
    }

    if (offset != NULL) {
        if (lseek(in_fd, initial_offset, SEEK_SET) == -1) {
            return -1;
        }
    }

    want_bytes = nbytes;

    while (1) {
        actual_bytes = write(out_fd, buf, want_bytes);
        if (actual_bytes == -1) {
            return -1;
        }
        
        if (actual_bytes == want_bytes) {
            break;
        }

        want_bytes -= actual_bytes;
    }

    if (offset != NULL) {
        *offset += nbytes;
    }
    
    return nbytes;
}

int main(int argc, char *argv[])
{
    char buf[BUFSIZE];
    ssize_t nbytes;
    int file_fd;
    int sock_fd[2];
    off_t offset;
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sock_fd) == -1) {
        perror("socketpair");
        exit(EXIT_FAILURE);
    }

    file_fd = open("file.txt", O_RDONLY);
    if (file_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    offset = 5;

    while ((nbytes = my_sendfile(sock_fd[1], file_fd, &offset, BUFSIZE))) {
        if (nbytes == -1) {
            perror("sendfile");
            exit(EXIT_FAILURE);
        }
    }

    printf("new offset value: %ld\n", offset);
    
    if (shutdown(sock_fd[1], SHUT_WR) == -1) {
        perror("shutdown");
        exit(EXIT_FAILURE);
    }
    
    if (close(sock_fd[1]) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

    while ((nbytes = read(sock_fd[0], buf, sizeof(buf)))) {
        if (nbytes == -1 && errno != EINTR) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        if (write(STDOUT_FILENO, buf, nbytes) != nbytes) {
            fprintf(stderr, "partial/failed write");
            exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}
