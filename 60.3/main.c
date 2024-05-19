#define _GNU_SOURCE
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
        exit(EXIT_FAILURE);
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

static void pty_read_tty_write_fork(int master_fd, int script_fd)
{
    char buf[BUF_SIZE];
    ssize_t num_read;
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        for (;;) {
            num_read = read(master_fd, buf, BUF_SIZE);
            if (num_read <= 0) {
                exit(EXIT_SUCCESS);
            }

            if (write(STDOUT_FILENO, buf, num_read) != num_read) {
                fprintf(stderr, "partial/failed write\n");
                exit(EXIT_FAILURE);
            }
            
            if (write(script_fd, buf, num_read) != num_read) {
                fprintf(stderr, "partial/failed write\n");
                exit(EXIT_FAILURE);
            }
        }

        exit(EXIT_SUCCESS);
    }
}

void sigchld_handler(int sig) {
}

static void tty_read_pty_write_fork(int master_fd, int script_fd)
{
    char buf[BUF_SIZE];
    ssize_t num_read;
    pid_t pid;
    struct sigaction sa;
    
    sigemptyset(&sa.sa_mask);
    sigaction(SIGCHLD, &sa, NULL);
    sa.sa_handler = sigchld_handler;

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        pty_read_tty_write_fork(master_fd, script_fd);
        
        for (;;) {
            num_read = read(STDIN_FILENO, buf, BUF_SIZE);
            if (num_read <= 0) {
                exit(EXIT_SUCCESS);
            }

            if (write(master_fd, buf, num_read) != num_read) {
                fprintf(stderr, "partial/failed write\n");
                exit(EXIT_FAILURE);
            }
        }

        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char *argv[])
{
    char slave_name[MAX_SNAME];
    char *shell;
    int master_fd, script_fd;
    struct winsize ws;
    pid_t child_pid;
    struct winsize new_ws;
    sigset_t blocked_mask;
    siginfo_t si;
    
    if (tcgetattr(STDIN_FILENO, &tty_orig) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
    
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == -1) {
        perror("ioctl");
        exit(EXIT_FAILURE);
    }

    child_pid = pty_fork(&master_fd, slave_name, MAX_SNAME, &tty_orig, &ws);
    if (child_pid == -1) {
        perror("pty_fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0) {
        shell = getenv("SHELL");
        if (shell == NULL || *shell == '\0') {
            shell = "/bin/sh";
        }

        execlp(shell, shell, (char *) NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }

    script_fd = open(
        (argc > 1) ? argv[1] : "typescript", 
        O_WRONLY | O_CREAT | O_TRUNC, 
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH
    );
    if (script_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    script_stream = fdopen(script_fd, "a");
    setbuf(script_stream, NULL);

    tty_set_raw(STDIN_FILENO, &tty_orig);

    tty_read_pty_write_fork(master_fd, script_fd);

    if (atexit(exit_func) != 0) {
        perror("atexit");
        exit(EXIT_FAILURE);
    }

    fprintf(script_stream, "Script started on %s", get_curr_time_str());

    sigemptyset(&blocked_mask);
    sigaddset(&blocked_mask, SIGWINCH);
    sigaddset(&blocked_mask, SIGCHLD);
    
    if (sigprocmask(SIG_SETMASK, &blocked_mask, NULL) == -1) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        if (sigwaitinfo(&blocked_mask, &si) == -1) {
            exit(EXIT_FAILURE);
        }

        if (si.si_signo == SIGWINCH) {
            if (ioctl(STDIN_FILENO, TIOCGWINSZ, &new_ws) == -1) {
                perror("ioctl");
                exit(EXIT_FAILURE);
            }

            if (ioctl(master_fd, TIOCSWINSZ, &new_ws) == -1) {
                perror("ioctl");
                exit(EXIT_FAILURE);
            }

            continue;
        }
        
        if (si.si_signo == SIGCHLD) {    
            exit(EXIT_SUCCESS);
        }

        fprintf(stderr, "unexpected interruption\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
