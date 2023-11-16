#define _GNU_SOURCE
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void my_abort(void)
{
    struct sigaction sa;
    sigset_t set;
    
    sigemptyset(&set);
    sigaddset(&set, SIGABRT);
    sigprocmask(SIG_UNBLOCK, &set, NULL);

    fflush(NULL);
    
    raise(SIGABRT);

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sigfillset(&sa.sa_mask);
    sigaction(SIGABRT, &sa, NULL);

    raise(SIGABRT);

    _exit(127);
}

int main(int argc, char *argv[])
{
    signal(SIGABRT, SIG_IGN);

    my_abort();

    return 0;
}