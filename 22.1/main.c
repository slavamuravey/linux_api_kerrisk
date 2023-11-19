#define _GNU_SOURCE
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

void handler(int s)
{
    int saved_errno = errno;
    char msg[] = "cont\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    errno = saved_errno;
}

int main(int argc, char *argv[])
{
    sigset_t set;
    struct sigaction sa;
    sigemptyset(&set);
    sigaddset(&set, SIGCONT);
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    sigaction(SIGCONT, &sa, NULL);
    sigprocmask(SIG_SETMASK, &set, NULL);

    printf("start sleeping...\n");
    sleep(10);
    
    printf("waked up\n");
    
    printf("starting some work...\n");
    sleep(5);
    printf("end work\n");
    sigprocmask(SIG_UNBLOCK, &set, NULL);

    return 0;
}