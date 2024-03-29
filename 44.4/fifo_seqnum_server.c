#define _GNU_SOURCE
#include <signal.h>
#include "../lib/fifo_seqnum.h"

void handler(int s)
{
    char err[] = "Unlink error\n";
    int saved_errno = errno;
    if (unlink(SERVER_FIFO) == -1) {
        write(STDOUT_FILENO, err, sizeof(err) - 1);
        exit(1);
    }
    errno = saved_errno;
    fflush(NULL);
    _exit(0);
}

int main(int argc, char *argv[])
{
    int serverFd, dummyFd, clientFd;
    char clientFifo[CLIENT_FIFO_NAME_LEN];
    struct request req;
    struct response resp;
    int seqNum = 0;
    umask(0);
    signal(SIGINT, handler);
    signal(SIGTERM, handler);

    if (mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST) {
        errExit("mkfifo %s", SERVER_FIFO);
    }
        
    serverFd = open(SERVER_FIFO, O_RDONLY);
    if (serverFd == -1) {
        errExit("open %s", SERVER_FIFO);
    }

    dummyFd = open(SERVER_FIFO, O_WRONLY);
    if (dummyFd == -1) {
        errExit("open %s", SERVER_FIFO);
    }

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        errExit("signal");
    }

    for (;;) {
        if (read(serverFd, &req, sizeof(struct request)) != sizeof(struct request)) {
            fprintf(stderr, "Error reading request; discarding\n");
            continue;
        }

        snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long) req.pid);
        clientFd = open(clientFifo, O_WRONLY);
        if (clientFd == -1) {
            errMsg("open %s", clientFifo);
            continue;
        }

        resp.seqNum = seqNum;
        if (write(clientFd, &resp, sizeof(struct response)) != sizeof(struct response)) {
            fprintf(stderr, "Error writing to FIFO %s\n", clientFifo);
        }
            
        if (close(clientFd) == -1) {
            errMsg("close");
        }

        seqNum += req.seqLen;
    }
}
