#define _GNU_SOURCE
#include <signal.h>
#include "../lib/fifo_seqnum.h"

int main(int argc, char *argv[])
{
    int serverFd, dummyFd, clientFd, seqFd;
    char clientFifo[CLIENT_FIFO_NAME_LEN];
    struct request req;
    struct response resp;
    struct stat statBuf;
    int seqNum;
    char *seqFilename = "./seq.dat";
    umask(0);
    if (mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST) {
        errExit("mkfifo %s", SERVER_FIFO);
    }

    seqFd = open(seqFilename, O_RDWR | O_CREAT | O_SYNC, S_IRUSR | S_IWUSR | S_IWGRP);
    if (seqFd == -1) {
        errExit("open %s", seqFilename);
    }
    
    if (fstat(seqFd, &statBuf)) {
        errExit("fstat %d", seqFd);
    }

    if (statBuf.st_size == 0) {
        seqNum = 0;
    } else {
        if (read(seqFd, &seqNum, sizeof(int)) == -1) {
            errExit("read");
        }
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

        if (lseek(seqFd, 0, SEEK_SET) == -1) {
            errMsg("lseek");
        }

        if (write(seqFd, &seqNum, sizeof(int)) != sizeof(int)) {
            fprintf(stderr, "Error writing to file %s\n", seqFilename);
        }
    }
}
