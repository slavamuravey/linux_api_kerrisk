#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/un.h>
#include "unix_sockets.h"
#include "../tlpi_hdr.h"

int unixAddressStr(const char *service, struct sockaddr_un *addr)
{
    if (addr == NULL || service == NULL) {
        errno = EINVAL;
        return -1;
    }

    memset(addr, 0, sizeof(struct sockaddr_un));
    addr->sun_family = AF_UNIX;

    if (strlen(service) > sizeof(addr->sun_path) - 1) {
        errno = ENAMETOOLONG;
        return -1;
    }
    
    strncpy(addr->sun_path, service, sizeof(addr->sun_path) - 1);

    return 0;
}

int unixConnect(const char *service, int type)
{
    struct sockaddr_un addr;
    int sfd;

    if (unixAddressStr(service, &addr) == -1) {
        return -1;
    }

    sfd = socket(AF_UNIX, type, 0);
    if (sfd == -1) {
        return -1;
    }

    if (connect(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        int savedErrno = errno;
        close(sfd);                      
        errno = savedErrno;
        return -1;
    }

    return sfd;
}

int unixBind(const char *service, int type)
{
    struct sockaddr_un addr;
    int sfd;

    if (remove(service) == -1 && errno != ENOENT) {
        return -1;
    }

    if (unixAddressStr(service, &addr) == -1) {
        return -1;
    }

    sfd = socket(AF_UNIX, type, 0);
    if (sfd == -1) {
        return -1;
    }

    if (bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        int savedErrno = errno;
        close(sfd);                      
        errno = savedErrno;
        return -1;
    }

    return sfd;
}
