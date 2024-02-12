#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define THREADS 1000

#define ONE_TIME_INIT_CONTROL_INITIALIZER { false, PTHREAD_MUTEX_INITIALIZER }

pthread_t ths[THREADS];

struct one_time_init_control_t {
    bool var;
    pthread_mutex_t mtx;
};

struct one_time_init_control_t control = ONE_TIME_INIT_CONTROL_INITIALIZER;

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

void Pthread_create(pthread_t *newthread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg)
{
    int s;
    s = pthread_create(newthread, attr, start_routine, arg);
    if (s != 0) {
        perror_num("pthread_create", s);
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

int one_time_init(struct one_time_init_control_t *control, void (*init)(void))
{
    Pthread_mutex_lock(&control->mtx);
    
    if (!control->var) {
        init();
        control->var = true;
    }
    
    Pthread_mutex_unlock(&control->mtx);
    
    return 0;
}

void init_func()
{
    printf("init\n");
}

void *func(void *args)
{
    one_time_init(&control, init_func);
    
    return NULL;
}

int main(int argc, char *argv[])
{
    int i;
    for (i = 0; i < THREADS; i++) {
        Pthread_create(&ths[i], NULL, func, NULL);
    }
    for (i = 0; i < THREADS; i++) {
        Pthread_join(ths[i], NULL);
    }
    
    return 0;
}