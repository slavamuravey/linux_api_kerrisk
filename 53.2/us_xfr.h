#include <sys/un.h>
#include <sys/socket.h>
#include "../lib/tlpi_hdr.h"

#define SV_SOCK_PATH "/tmp/us_xfr"
#define ABSTRACT_SOCK_NAME "test"

#define BUF_SIZE 100