#define _GNU_SOURCE
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
    pid_t pid;
    int req_fd[2], res_fd[2];
    int read_req_fd, write_req_fd, read_res_fd, write_res_fd;
    ssize_t nbytes;
    char buf[PIPE_BUF];

    if (pipe(req_fd) == -1) {
        perror("pipe");
        exit(1);
    }

    if (pipe(res_fd) == -1) {
        perror("pipe");
        exit(1);
    }

    read_req_fd = req_fd[0];
    write_req_fd = req_fd[1];

    read_res_fd = res_fd[0];
    write_res_fd = res_fd[1];
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        if (close(read_res_fd) == -1) {
            perror("close");
            exit(1);
        }

        if (close(write_req_fd) == -1) {
            perror("close");
            exit(1);
        }

        while ((nbytes = read(read_req_fd, buf, sizeof(buf)))) {
            ssize_t write_nbytes;
            int i;
            if (nbytes == -1) {
                if (errno != EINTR) {
                    perror("read");
                    exit(1);
                }

                continue;
            }

            for (i = 0; i < nbytes; i++) {
                buf[i] = toupper(buf[i]);
            }

            write_nbytes = write(write_res_fd, buf, nbytes);
            if (write_nbytes == -1) {
                perror("write");
                exit(1);
            }
            
            if (write_nbytes != nbytes) {
                fprintf(stderr, "incomplete write.");
                exit(1);
            }
        }

        if (close(read_req_fd) == -1) {
            perror("close");
            exit(1);
        }

        if (close(write_res_fd) == -1) {
            perror("close");
            exit(1);
        }
        
        exit(0);
    }

    if (close(read_req_fd) == -1) {
        perror("close");
        exit(1);
    }

    if (close(write_res_fd) == -1) {
        perror("close");
        exit(1);
    }

    while ((nbytes = read(STDIN_FILENO, buf, sizeof(buf)))) {
        ssize_t write_nbytes, read_nbytes;
        if (nbytes == -1) {
            if (errno != EINTR) {
                perror("read");
                exit(1);
            }

            continue;
        }

        write_nbytes = write(write_req_fd, buf, nbytes);
        if (write_nbytes == -1) {
            perror("write");
            exit(1);
        }
        
        if (write_nbytes != nbytes) {
            fprintf(stderr, "incomplete write.");
            exit(1);
        }

        while (1) {
            read_nbytes = read(read_res_fd, buf, sizeof(buf));
            if (read_nbytes == -1) {
                if (errno != EINTR) {
                    perror("read");
                    exit(1);
                }

                continue;
            }

            break;
        }

        write_nbytes = write(STDOUT_FILENO, buf, nbytes);
        if (write_nbytes == -1) {
            perror("write");
            exit(1);
        }
        
        if (write_nbytes != nbytes) {
            fprintf(stderr, "incomplete write.");
            exit(1);
        }
    }

    if (close(read_res_fd) == -1) {
        perror("close");
        exit(1);
    }

    if (close(write_req_fd) == -1) {
        perror("close");
        exit(1);
    }

    if (wait(NULL) == -1) {
        perror("wait");
        exit(1);
    }

    return 0;
}
