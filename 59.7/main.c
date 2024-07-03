#define _GNU_SOURCE
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <termios.h>
#include "../lib/tty/tty.h"
#include "../lib/tlpi_hdr.h"

static volatile sig_atomic_t gotSigio = 0;

static void sigioHandler(int sig, siginfo_t *si, void *ucontext)
{
    gotSigio = 1;
    printf("si_fd: %d, si_code: %d\n", si->si_fd, si->si_code);
}

int main(int argc, char *argv[])
{
    int flags, j, cnt;
    struct termios origTermios;
    char ch;
    struct sigaction sa;
    Boolean done;
    int RT_SIGIO;
    
    RT_SIGIO = SIGRTMIN + 5;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sa.sa_sigaction = sigioHandler;
    if (sigaction(RT_SIGIO, &sa, NULL) == -1) {
        errExit("sigaction");
    }

    if (fcntl(STDIN_FILENO, F_SETOWN, getpid()) == -1) {
        errExit("fcntl(F_SETOWN)");
    }

    if (fcntl(STDIN_FILENO, F_SETSIG, RT_SIGIO) == -1) {
        errExit("fcntl");
    }

    flags = fcntl(STDIN_FILENO, F_GETFL);
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_ASYNC | O_NONBLOCK) == -1) {
        errExit("fcntl(F_SETFL)");
    }

    if (tty_set_cbreak(STDIN_FILENO, &origTermios) == -1) {
        errExit("tty_set_cbreak");
    }

    for (done = FALSE, cnt = 0; !done ; cnt++) {
        for (j = 0; j < 100000000; j++) {
            continue;
        }

        if (gotSigio) {
            gotSigio = 0;

            while (read(STDIN_FILENO, &ch, 1) > 0 && !done) {
                printf("cnt=%d; read %c\n", cnt, ch);
                done = ch == '#';
            }
        }
    }

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &origTermios) == -1) {
        errExit("tcsetattr");
    }
        
    exit(EXIT_SUCCESS);
}
