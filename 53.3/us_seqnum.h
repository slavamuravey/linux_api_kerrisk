#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SV_SOCK_PATH "/tmp/53.3-server.sock"
#define CL_SOCK_PATH_TPL "/tmp/53.3-client.sock.%ld"
#define CL_SOCK_PATH_LEN (sizeof(CL_SOCK_PATH_TPL) + 20)

struct request {                
    pid_t pid;                  
    int seq_len;                
};

struct response {               
    int seq_num;                 
};

#endif