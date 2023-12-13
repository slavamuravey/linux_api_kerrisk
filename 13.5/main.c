#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define BUF_SIZE 4096

int main(int argc, char *argv[])
{
    char *filename;
    ssize_t nlines;
    int fd;
    ssize_t nread;
    char buf[BUF_SIZE];
    off_t *pos_buf;
    
    off_t pos = 0;
    size_t lines_counter = 0;
    
    if (argc < 3) {
        fprintf(stderr, "too few arguments.\n");
        exit(EXIT_FAILURE);
    }

    filename = argv[1];
    nlines = strtol(argv[2], NULL, 10);
    if (errno) {
        perror("strtol");
        exit(EXIT_FAILURE);
    }

    if (nlines < 0) {
        nlines = -nlines;
    }

    nlines++;

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    pos_buf = malloc(nlines * sizeof(off_t));
    pos_buf[0] = 0;

    while ((nread = read(fd, buf, BUF_SIZE))) {
        int i;

        if (nread == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        for (i = 0; i < nread; i++) {
            pos++;
            if (buf[i] == '\n') {
                lines_counter++;
                pos_buf[lines_counter % nlines] = pos;
            }
        }
    }

    lseek(fd, pos_buf[(lines_counter + 1) % nlines], SEEK_SET);

    free(pos_buf);

    while ((nread = read(fd, buf, BUF_SIZE))) {
        if (nread == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        write(STDOUT_FILENO, buf, nread);
    }

    return 0;
}
