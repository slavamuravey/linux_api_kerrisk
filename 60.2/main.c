#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <termios.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../lib/tty/tty.h" 

#define BUF_SIZE 256
#define MAX_SNAME 1000

struct termios tty_orig;

static void tty_reset(void)
{
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty_orig) == -1) {
        perror("tcsetattr");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    char slave_name[MAX_SNAME];
    char *shell;
    int master_fd, script_fd;
    struct winsize ws;
    fd_set in_fds;
    char buf[BUF_SIZE];
    ssize_t num_read;
    pid_t child_pid;

    /* Retrieve the attributes of terminal on which we are started */

    if (tcgetattr(STDIN_FILENO, &tty_orig) == -1) {
        perror("tcgetattr");
        exit(1);
    }
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0) {
        perror("ioctl");
        exit(1);
    }

    child_pid = pty_fork(&master_fd, slave_name, MAX_SNAME, &tty_orig, &ws);
    if (child_pid == -1) {
        perror("pty_fork");
        exit(1);
    }

    if (child_pid == 0) {
        shell = getenv("SHELL");
        if (shell == NULL || *shell == '\0') {
            shell = "/bin/sh";
        }

        execlp(shell, shell, (char *) NULL);
        perror("execlp");
        exit(1);
    }

    script_fd = open((
        argc > 1) ? argv[1] : "typescript", 
        O_WRONLY | O_CREAT | O_TRUNC, 
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
    );
    if (script_fd == -1) {
        perror("open");
        exit(1);
    }

    tty_set_raw(STDIN_FILENO, &tty_orig);

    if (atexit(tty_reset) != 0) {
        perror("atexit");
        exit(1);
    }

    for (;;) {
        FD_ZERO(&in_fds);
        FD_SET(STDIN_FILENO, &in_fds);
        FD_SET(master_fd, &in_fds);

        if (select(master_fd + 1, &in_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

        if (FD_ISSET(STDIN_FILENO, &in_fds)) {
            num_read = read(STDIN_FILENO, buf, BUF_SIZE);
            if (num_read <= 0) {
                exit(EXIT_SUCCESS);
            }

            if (write(master_fd, buf, num_read) != num_read) {
                fprintf(stderr, "partial/failed write\n");
                exit(1);
            }
        }

        if (FD_ISSET(master_fd, &in_fds)) {
            num_read = read(master_fd, buf, BUF_SIZE);
            if (num_read <= 0) {
                exit(EXIT_SUCCESS);
            }

            if (write(STDOUT_FILENO, buf, num_read) != num_read) {
                fprintf(stderr, "partial/failed write\n");
                exit(1);
            }
            if (write(script_fd, buf, num_read) != num_read) {
                fprintf(stderr, "partial/failed write\n");
                exit(1);
            }
        }
    }
}
