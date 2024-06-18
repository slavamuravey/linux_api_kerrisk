#include "us_xfr.h"

int main(int argc, char *argv[])
{
    int sfd;
    ssize_t numRead;
    char buf[BUF_SIZE];

    sfd = unixConnect(SV_SOCK_PATH, SOCK_STREAM);
    if (sfd == -1) {
        errExit("unixConnect");
    }
        
    /* Copy stdin to socket */

    while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
        if (write(sfd, buf, numRead) != numRead) {
            fatal("partial/failed write");
        }
    }

    if (numRead == -1) {
        errExit("read");
    }

    exit(EXIT_SUCCESS);         /* Closes our socket; server sees EOF */
}