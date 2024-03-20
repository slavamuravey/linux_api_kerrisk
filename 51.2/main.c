#define _DEFAULT_SOURCE
#include <sys/file.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/mman.h>

int main(int argc, char *argv[])
{
    pid_t pid;
    int fd1, fd2;
    sem_t *sem1, *sem2;

    sem1 = mmap(NULL, sizeof(sem1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sem1 == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    sem2 = mmap(NULL, sizeof(sem2), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sem2 == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    if (sem_init(sem1, 1, 0) == -1) {
        perror("sem_init");
        exit(1);
    }

    if (sem_init(sem2, 1, 0) == -1) {
        perror("sem_init");
        exit(1);
    }
    
    fd1 = open("file1.txt", O_RDWR);
    if (fd1 == -1) {
        perror("open");
        exit(1);
    }

    fd2 = open("file2.txt", O_RDWR);
    if (fd2 == -1) {
        perror("open");
        exit(1);
    }
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        fd1 = open("file1.txt", O_RDWR);
        if (fd1 == -1) {
            perror("open");
            exit(1);
        }

        fd2 = open("file2.txt", O_RDWR);
        if (fd2 == -1) {
            perror("open");
            exit(1);
        }

        if (flock(fd2, LOCK_EX) == -1) {
            perror("flock");
            exit(1);
        }

        printf("[%d] file2.txt locked\n", getpid());

        if (sem_post(sem1) == -1) {
            perror("sem_post");
            exit(1);
        }

        if (sem_wait(sem2) == -1) {
            perror("sem_wait");
            exit(1);
        }

        if (flock(fd1, LOCK_EX) == -1) {
            perror("flock");
            exit(1);
        }

        printf("[%d] file1.txt locked\n", getpid());

        exit(0);
    }

    if (flock(fd1, LOCK_EX) == -1) {
        perror("flock");
        exit(1);
    }

    if (sem_post(sem2) == -1) {
        perror("sem_post");
        exit(1);
    }

    if (sem_wait(sem1) == -1) {
        perror("sem_wait");
        exit(1);
    }

    printf("[%d] file1.txt locked\n", getpid());

    if (flock(fd2, LOCK_EX) == -1) {
        perror("flock");
        exit(1);
    }

    printf("[%d] file2.txt locked\n", getpid());

    return 0;
}