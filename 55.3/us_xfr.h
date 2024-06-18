#include <sys/un.h>
#include <sys/socket.h>
#include "../lib/tlpi_hdr.h"
#include "../lib/socket/unix_sockets.h"

#define SV_SOCK_PATH "/tmp/us_xfr"

#define BUF_SIZE 100