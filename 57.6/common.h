#ifndef COMMON_H
#define COMMON_H

#include <inttypes.h>

typedef char uuid_str_t[37];

struct urg_handshake_req {
    uint16_t port;
    uuid_str_t con_id;
};

struct urg_handshake_res {
    uuid_str_t con_id;
};

struct connection {
    int sock;
    int urg_sock;
};

long port_from_str(const char *str);

#endif