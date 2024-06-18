#include "us_xfr.h"
#define BACKLOG 5

int main(int argc, char *argv[])
{
    int sfd, cfd;
    ssize_t numRead;
    char buf[BUF_SIZE];

    sfd = unixBind(SV_SOCK_PATH, SOCK_STREAM);
    if (sfd == -1) {
        errExit("unixBind");
    }
    
    if (listen(sfd, BACKLOG) == -1) {
        errExit("listen");
    }
    
    for (;;) {          /* Handle client connections iteratively */

        /* Accept a connection. The connection is returned on a new
           socket, 'cfd'; the listening socket ('sfd') remains open
           and can be used to accept further connections. */

        cfd = accept(sfd, NULL, NULL);
        if (cfd == -1) {
            errExit("accept");
        }
            
        /* Transfer data from connected socket to stdout until EOF */

        while ((numRead = read(cfd, buf, BUF_SIZE)) > 0) {
            if (write(STDOUT_FILENO, buf, numRead) != numRead) {
                fatal("partial/failed write");
            }
        }

        if (numRead == -1) {
            errExit("read");
        }
            
        if (close(cfd) == -1) {
            errMsg("close");
        }
    }
}