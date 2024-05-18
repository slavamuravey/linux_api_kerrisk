#define _POSIX_C_SOURCE 1
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <termios.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include "../lib/tty/tty.h" 

#define BUF_SIZE 256
#define MAX_SNAME 1000

struct termios tty_orig;
static FILE *script_stream;

static void tty_reset(void)
{
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty_orig) == -1) {
        perror("tcsetattr");
        exit(1);
    }
}

static char *get_curr_time_str()
{
    time_t rawtime;
    struct tm *timeinfo;

    rawtime = time(NULL);
    timeinfo = localtime(&rawtime);
    
    return asctime(timeinfo);
}

static void exit_func(void)
{
    fprintf(script_stream, "Script done on %s", get_curr_time_str());
    tty_reset();
}

void sigwinch_handler(int sig) {
}

int main(int argc, char *argv[])
{
    char slave_name[MAX_SNAME];
    char *shell;
    int master_fd, script_fd;
    struct winsize ws, new_ws;
    fd_set in_fds;
    char buf[BUF_SIZE];
    ssize_t num_read;
    pid_t child_pid;
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigwinch_handler;
    
    if (sigaction(SIGWINCH, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    
    if (tcgetattr(STDIN_FILENO, &tty_orig) == -1) {
        perror("tcgetattr");
        exit(1);
    }
    
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == -1) {
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

    script_fd = open(
        (argc > 1) ? argv[1] : "typescript", 
        O_WRONLY | O_CREAT | O_TRUNC, 
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
    );
    if (script_fd == -1) {
        perror("open");
        exit(1);
    }

    script_stream = fdopen(script_fd, "a");
    setbuf(script_stream, NULL);

    tty_set_raw(STDIN_FILENO, &tty_orig);

    if (atexit(exit_func) != 0) {
        perror("atexit");
        exit(1);
    }

    fprintf(script_stream, "Script started on %s", get_curr_time_str());

    for (;;) {
        int sd;
        FD_ZERO(&in_fds);
        FD_SET(STDIN_FILENO, &in_fds);
        FD_SET(master_fd, &in_fds);

        sd = select(master_fd + 1, &in_fds, NULL, NULL, NULL);
        if (sd == -1) {
            if (errno == EINTR) {
                if (ioctl(STDIN_FILENO, TIOCGWINSZ, &new_ws) == -1) {
                    perror("ioctl");
                    exit(1);
                }

                if (ioctl(master_fd, TIOCSWINSZ, &new_ws) == -1) {
                    perror("ioctl");
                    exit(1);
                }
                
                continue;
            }
            
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
