#ifndef READ_LINE_H
#define READ_LINE_H

#include <sys/types.h>

#define RL_MAX_BUF 10

struct readLineBuf {
    int fd;
    size_t len;
    int next;
    char buf[RL_MAX_BUF];
};

ssize_t readLine(int fd, void *buffer, size_t n);
void readLineBufInit(int fd, struct readLineBuf *rlbuf);
ssize_t readLineBuf(struct readLineBuf *rlbuf, char *buffer, size_t n);

#endif