#define _GNU_SOURCE
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/signal_functions.h"

int ready_threads;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void perror_num(const char *msg, int err)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(err));
}

void Pthread_mutex_lock(pthread_mutex_t *mutex)
{
    int s;
    s = pthread_mutex_lock(mutex);
    if (s != 0) {
        perror_num("pthread_mutex_lock", s);
        exit(1);
    }
}

void Pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    int s;
    s = pthread_mutex_unlock(mutex);
    if (s != 0) {
        perror_num("pthread_mutex_unlock", s);
        exit(1);
    }
}

void Pthread_cond_signal(pthread_cond_t *cond)
{
    int s;
    s = pthread_cond_signal(cond);
    if (s != 0) {
        perror_num("pthread_cond_signal", s);
        exit(1);
    }
}

void Pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    int s;
    s = pthread_cond_wait(cond, mutex);
    if (s != 0) {
        perror_num("pthread_cond_wait", s);
        exit(1);
    }
}

void Pthread_create(pthread_t *newthread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg)
{
    int s;
    s = pthread_create(newthread, attr, start_routine, arg);
    if (s != 0) {
        perror_num("pthread_create", s);
        exit(1);
    }
}

void Pthread_kill(pthread_t threadid, int signo)
{
    int s;
    s = pthread_kill(threadid, signo);
    if (s != 0) {
        perror_num("pthread_kill", s);
        exit(1);
    }
}

void Pthread_join(pthread_t th, void **thread_return)
{
    int s;
    s = pthread_join(th, thread_return);
    if (s != 0) {
        perror_num("pthread_join", s);
        exit(1);
    }
}

void Pthread_sigmask(int how, const __sigset_t *newmask, __sigset_t *oldmask)
{
    int s;
    s = pthread_sigmask(how, newmask, oldmask);
    if (s != 0) {
        perror_num("pthread_sigmask", s);
        exit(1);
    }
}

void handler(int s)
{
}

void *func(void *args)
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    Pthread_sigmask(SIG_SETMASK, &set, NULL);
    
    Pthread_mutex_lock(&mtx);
    printf("%d is ready...", gettid());
    printPendingSigs(stdout, "pending signals: ");
    ready_threads++;
    Pthread_mutex_unlock(&mtx);
    
    Pthread_cond_signal(&cond);

    pause();
    
    Pthread_mutex_lock(&mtx);
    printf("[%d] ", gettid());
    printPendingSigs(stdout, "pending signals: ");
    Pthread_mutex_unlock(&mtx);
    
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t th1, th2;
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    
    sigaction(SIGTERM, &sa, NULL);

    Pthread_create(&th1, NULL, func, NULL);
    Pthread_create(&th2, NULL, func, NULL);

    Pthread_mutex_lock(&mtx);

    while (ready_threads < 2) {
        Pthread_cond_wait(&cond, &mtx);
    }

    Pthread_mutex_unlock(&mtx);

    Pthread_kill(th1, SIGUSR1);
    Pthread_kill(th2, SIGUSR2);
    Pthread_kill(th1, SIGTERM);
    Pthread_kill(th2, SIGTERM);

    Pthread_join(th1, NULL);
    Pthread_join(th2, NULL);

    return 0;
}