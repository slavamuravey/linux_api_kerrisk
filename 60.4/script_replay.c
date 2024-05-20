#define _XOPEN_SOURCE 500
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 256

size_t my_getline(char **line, size_t *buffer, int fd)
{
    int read_chars;
    char buf[BUF_SIZE];
    off_t current_pos;
    ssize_t count;
    current_pos = lseek(fd, 0, SEEK_CUR);
    read_chars = 0;

    while ((count = read(fd, buf, sizeof(buf)))) {
        int i;
        for (i = 0; i < count; i++) {
            read_chars++;

            if (read_chars > *buffer - 1) {
                *buffer *= 2;
                *line = realloc(*line, *buffer);
                if (*line == NULL) {
                    perror("realloc");
                    exit(EXIT_FAILURE);
                }
            }

            (*line)[read_chars - 1] = buf[i];

            if (buf[i] == '\n') {
                lseek(fd, current_pos + read_chars, SEEK_SET);
                goto end_loop;
            }
        }
    }
end_loop:

    if (read_chars == 0) {
        return 0;
    }

    (*line)[read_chars] = '\0';

    return read_chars;
}

int main(int argc, char *argv[])
{
    int script_fd, script_timed_fd;
    size_t bufsize;
    ssize_t num_read;
    char *line;
    char timestamp_s_str[BUF_SIZE];
    char timestamp_u_str[BUF_SIZE];
    char num_read_str[BUF_SIZE];
    char buf[BUF_SIZE];
    unsigned long sec, usec, nread;
    
    script_fd = open(
        (argc > 1) ? argv[1] : "typescript", 
        O_RDONLY, 
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
    );
    if (script_fd == -1) {
        perror("open");
        exit(1);
    }

    script_timed_fd = open(
        (argc > 2) ? argv[2] : "typescript.timed", 
        O_RDONLY, 
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
    );
    if (script_timed_fd == -1) {
        perror("open");
        exit(1);
    }

    for (;;) {
        int i;
        num_read = read(script_fd, buf, BUF_SIZE);
        if (num_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        for (i = 0; i < num_read; i++) {
            if (buf[i] == '\n') {
                if (lseek(script_fd, i + 1, SEEK_SET) == -1) {
                    perror("lseek");
                    exit(EXIT_FAILURE);
                }
                goto loop_break;
            }
        }
    }

loop_break:

    bufsize = BUF_SIZE;
    line = malloc(sizeof(char) * bufsize);

    while ((num_read = my_getline(&line, &bufsize, script_timed_fd))) {
        int i;
        int pos;

        memset(timestamp_s_str, 0, sizeof(timestamp_s_str));
        memset(timestamp_u_str, 0, sizeof(timestamp_u_str));
        memset(num_read_str, 0, sizeof(num_read_str));
        pos = 0;

        for (i = 0; i < strlen(line); i++) {
            if (line[i] == '.') {
                strncpy(timestamp_s_str, line, i);
                pos = i + 1;
            }
            if (line[i] == ' ') {
                strncpy(timestamp_u_str, line + pos, i - pos);
                pos = i + 1;
            }
            if (line[i] == '\n') {
                strncpy(num_read_str, line + pos, i - pos);
            }
        }

        sec = strtoul(timestamp_s_str, NULL, 10);
        usec = strtoul(timestamp_u_str, NULL, 10);
        nread = strtoul(num_read_str, NULL, 10);

        usleep(sec * 1e6 + usec);

        num_read = read(script_fd, buf, nread);
        if (num_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        if (write(STDOUT_FILENO, buf, num_read) != num_read) {
            fprintf(stderr, "partial/failed write\n");
            exit(EXIT_FAILURE);
        }
    }
    
    return 0;
}