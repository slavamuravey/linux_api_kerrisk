#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include <libgen.h>

#define PATH1 "/some1/path1/file1.txt"
#define PATH2 "/some2/path2/file2.txt"

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

void Pthread_key_create(pthread_key_t *key, void (*destr_function) (void *))
{
    int s;
    s = pthread_key_create(key, destr_function);
    if (s != 0) {
        perror_num("pthread_key_create", s);
        exit(1);
    }
}

void Pthread_once(pthread_once_t *once_control, void (*init_routine) (void))
{
    int s;
    s = pthread_once(once_control, init_routine);
    if (s != 0) {
        perror_num("pthread_once", s);
        exit(1);
    }
}

void Pthread_setspecific(pthread_key_t key, const void *pointer)
{
    int s;
    s = pthread_setspecific(key, pointer);
    if (s != 0) {
        perror_num("pthread_setspecific", s);
        exit(1);
    }
}

pthread_once_t my_basename_once = PTHREAD_ONCE_INIT;
pthread_once_t my_dirname_once = PTHREAD_ONCE_INIT;
pthread_key_t my_basename_key;
pthread_key_t my_dirname_key;
pthread_mutex_t basename_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t dirname_mtx = PTHREAD_MUTEX_INITIALIZER;

void destructor(void *buf)
{
    free(buf);
}

void create_my_basename_key()
{
    Pthread_key_create(&my_basename_key, destructor);
}

void create_my_dirname_key()
{
    Pthread_key_create(&my_dirname_key, destructor);
}

char *my_basename(char *path)
{
    char *buf;
    Pthread_once(&my_basename_once, create_my_basename_key);

    buf = pthread_getspecific(my_basename_key);
    if (buf == NULL) {
        buf = malloc(PATH_MAX);
        if (buf == NULL) {
            perror("malloc");
            exit(1);
        }

        Pthread_setspecific(my_basename_key, buf);
    }

    Pthread_mutex_lock(&basename_mtx);
    strcpy(buf, basename(path));
    Pthread_mutex_unlock(&basename_mtx);

    return buf;
}

char *my_dirname(char *path)
{
    char *buf;
    Pthread_once(&my_dirname_once, create_my_dirname_key);

    buf = pthread_getspecific(my_dirname_key);
    if (buf == NULL) {
        buf = malloc(PATH_MAX);
        if (buf == NULL) {
            perror("malloc");
            exit(1);
        }

        Pthread_setspecific(my_dirname_key, buf);
    }

    Pthread_mutex_lock(&dirname_mtx);
    strcpy(buf, dirname(path));
    Pthread_mutex_unlock(&dirname_mtx);

    return buf;
}

void *func(void *args)
{
    char *name, *path;
    path = strdup(PATH2);
    name = my_basename(path);
    printf("child thread thread basename: %s\n", name);
    name = my_dirname(path);
    printf("child thread thread dirname: %s\n", name);
    free(path);
    
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t th;
    char *basename_str, *dirname_str, *path;
    path = strdup(PATH1);

    basename_str = my_basename(path);
    dirname_str = my_dirname(path);

    Pthread_create(&th, NULL, func, NULL);
    Pthread_join(th, NULL);

    printf("main thread basename: %s\n", basename_str);
    printf("main thread dirname: %s\n", dirname_str);
    free(path);
    
    return 0;
}