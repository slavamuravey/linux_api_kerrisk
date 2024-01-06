#define _GNU_SOURCE
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

unsigned int my_alarm(unsigned int seconds)
{
    struct itimerval new_value, old_value;
    memset(&new_value, 0, sizeof(new_value));
    new_value.it_value.tv_sec = seconds;
    setitimer(ITIMER_REAL, &new_value, &old_value);
    
    return old_value.it_value.tv_sec + (old_value.it_value.tv_usec > 0 ? 1 : 0);
}

void sigalrm_handler(int s)
{
    char msg[] = "alarm!\n";
    int saved_errno = errno;
    write(STDOUT_FILENO, msg, strlen(msg));
    errno = saved_errno;
}

int main(int argc, char *argv[])
{
    struct sigaction act;
    if (sigemptyset(&act.sa_mask) == -1) {
        perror("sigemptyset");
        exit(EXIT_FAILURE);
    }
    act.sa_flags = SA_RESTART;
    act.sa_handler = sigalrm_handler;
    if (sigaction(SIGALRM, &act, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    
    my_alarm(3);

    pause();

    return 0;
}