#define _POSIX_C_SOURCE 200112L
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "is_seqnum.h"
#include "../lib/socket/inet_sockets.h"

int main(int argc, char *argv[])
{
    char *reqLenStr;                    /* Requested length of sequence */
    char seqNumStr[INT_LEN];            /* Start of granted sequence */
    int cfd;
    ssize_t numRead;
    struct readLineBuf rlbuf;

    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        usageErr("%s server-host [sequence-len]\n", argv[0]);
    }

    cfd = inetConnect(argv[1], PORT_NUM, SOCK_STREAM);
    if (cfd == -1) {
        errExit("inetConnect");
    }

    /* Send requested sequence length, with terminating newline */

    reqLenStr = (argc > 2) ? argv[2] : "1";
    if (write(cfd, reqLenStr, strlen(reqLenStr)) != strlen(reqLenStr)) {
        fatal("Partial/failed write (reqLenStr)");
    }
    if (write(cfd, "\n", 1) != 1) {
        fatal("Partial/failed write (newline)");
    }

    /* Read and display sequence number returned by server */

    readLineBufInit(cfd, &rlbuf);
    numRead = readLineBuf(&rlbuf, seqNumStr, INT_LEN);
    if (numRead == -1) {
        errExit("readLine");
    }
    if (numRead == 0) {
        fatal("Unexpected EOF from server");
    }

    printf("Sequence number: %s", seqNumStr);   /* Includes '\n' */

    exit(EXIT_SUCCESS);                         /* Closes 'cfd' */
}