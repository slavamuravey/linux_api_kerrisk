#include <stdlib.h>
#include <sys/epoll.h>
#include <stdio.h>

#define MAX_EVENTS 5

int main(int argc, char *argv[])
{
    int epfd, ready;
    struct epoll_event evlist[MAX_EVENTS];
    
    epfd = epoll_create(MAX_EVENTS);
    if (epfd == -1) {
        perror("epoll_create");
        exit(EXIT_FAILURE);
    }

    ready = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
    if (ready == -1) {
        perror("epoll_wait");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
