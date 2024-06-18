#ifndef UNIX_SOCKETS_H
#define UNIX_SOCKETS_H

#include <sys/socket.h>

int unixConnect(const char *service, int type);

int unixBind(const char *service, int type);

int unixAddressStr(const char *service, struct sockaddr_un *addr);

#endif