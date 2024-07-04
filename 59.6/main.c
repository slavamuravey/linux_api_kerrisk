#include <stdlib.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_EVENTS 5

int main(int argc, char *argv[])
{
    int epfd, ready;
    struct epoll_event evlist[MAX_EVENTS];
    struct epoll_event ev;
    int fd1[2], fd2[2], fd3[2];
    int j;

    if (pipe(fd1) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    if (pipe(fd2) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    if (pipe(fd3) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    
    epfd = epoll_create(MAX_EVENTS);
    if (epfd == -1) {
        perror("epoll_create");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLOUT;
    ev.data.fd = fd1[1];
    printf("fd1: %d\n", fd1[1]);

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd1[1], &ev) == -1) {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLOUT;
    ev.data.fd = fd2[1];
    printf("fd2: %d\n", fd2[1]);

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd2[1], &ev) == -1) {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLOUT;
    ev.data.fd = fd3[1];
    printf("fd3: %d\n", fd3[1]);

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd3[1], &ev) == -1) {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }

    ready = epoll_wait(epfd, evlist, 2, -1);
    if (ready == -1) {
        perror("epoll_wait");
        exit(EXIT_FAILURE);
    }

    for (j = 0; j < ready; j++) {
        if (evlist[j].events & EPOLLOUT) {
            printf("[%d]fd: %d\n", j, evlist[j].data.fd);
        }
    }

    ready = epoll_wait(epfd, evlist, 2, -1);
    if (ready == -1) {
        perror("epoll_wait");
        exit(EXIT_FAILURE);
    }

    for (j = 0; j < ready; j++) {
        if (evlist[j].events & EPOLLOUT) {
            printf("[%d]fd: %d\n", j, evlist[j].data.fd);
        }
    }

    return EXIT_SUCCESS;
}
