#define _XOPEN_SOURCE 500
#include <time.h>
#include <sys/select.h>
#include "../lib/tlpi_hdr.h"

int main(int argc, char *argv[])
{
    int numPipes, ready, randPipe, numWrites, j;
    int (*pfds)[2];
    fd_set in_fds;
    int fd_max;

    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        usageErr("%s num-pipes [num-writes]\n", argv[0]);
    }

    numPipes = getInt(argv[1], GN_GT_0, "num-pipes");
    numWrites = (argc > 2) ? getInt(argv[2], GN_GT_0, "num-writes") : 1;

    pfds = calloc(numPipes, sizeof(int [2]));
    if (pfds == NULL) {
        errExit("calloc");
    }
        
    for (j = 0; j < numPipes; j++) {
        if (pipe(pfds[j]) == -1) {
            errExit("pipe %d", j);
        }
    }

    srandom((int) time(NULL));
    for (j = 0; j < numWrites; j++) {
        randPipe = random() % numPipes;
        printf("Writing to fd: %3d (read fd: %3d)\n", pfds[randPipe][1], pfds[randPipe][0]);
        if (write(pfds[randPipe][1], "a", 1) == -1) {
            errExit("write %d", pfds[randPipe][1]);
        }
    }

    FD_ZERO(&in_fds);

    fd_max = -1;

    for (j = 0; j < numPipes; j++) {
        if (pfds[j][0] > fd_max) {
            fd_max = pfds[j][0];
        }
        
        FD_SET(pfds[j][0], &in_fds);
    }

    ready = select(fd_max + 1, &in_fds, NULL, NULL, NULL);
    if (ready == -1) {
        errExit("select");
    }

    printf("select() returned: %d\n", ready);

    for (j = 0; j < numPipes; j++) {
        if (FD_ISSET(pfds[j][0], &in_fds)) {
            printf("Readable: %3d\n", pfds[j][0]);
        }
    }

    exit(EXIT_SUCCESS);
}
