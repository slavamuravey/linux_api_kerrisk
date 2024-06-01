#define _GNU_SOURCE
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/mman.h>

void handler(int sig)
{
    char *signal_type_str;
    signal_type_str = sig < SIGRTMIN ? "standard signal" : "rt-signal";
    printf("%s: %d\n", signal_type_str, sig);
}

int main(int argc, char *argv[])
{
    pid_t pid;
    union sigval sv;
    sem_t *sem_ready, *sem_signal_sent;

    sem_ready = mmap(NULL, sizeof(sem_ready), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sem_ready == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    sem_signal_sent = mmap(NULL, sizeof(sem_signal_sent), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sem_signal_sent == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    if (sem_init(sem_ready, 1, 0) == -1) {
        perror("sem_init");
        exit(1);
    }

    if (sem_init(sem_signal_sent, 1, 0) == -1) {
        perror("sem_init");
        exit(1);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        struct sigaction sa;
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGRTMIN);
        sigaddset(&mask, SIGUSR1);

        sa.sa_handler = handler;
        if (sigaction(SIGRTMIN, &sa, NULL) == -1) {
            perror("sigaction");
            exit(1);
        }
        
        if (sigaction(SIGUSR1, &sa, NULL) == -1) {
            perror("sigaction");
            exit(1);
        }

        if (sigprocmask(SIG_SETMASK, &mask, NULL)) {
            perror("sigprocmask");
            exit(1);
        }

        if (sem_post(sem_ready) == -1) {
            perror("sem_post");
            exit(1);
        }

        if (sem_wait(sem_signal_sent) == -1) {
            perror("sem_wait");
            exit(1);
        }

        if (sigprocmask(SIG_UNBLOCK, &mask, NULL)) {
            perror("sigprocmask");
            exit(1);
        }

        exit(0);
    }

    if (sem_wait(sem_ready) == -1) {
        perror("sem_wait");
        exit(1);
    }

    if (kill(pid, SIGUSR1) == -1) {
        perror("kill");
        exit(1);
    }
    
    if (sigqueue(pid, SIGRTMIN, sv)) {
        perror("sigqueue");
        exit(1);
    }

    if (sem_post(sem_signal_sent) == -1) {
        perror("sem_post");
        exit(1);
    }

    return 0;
}