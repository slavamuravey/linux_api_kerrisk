#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "../lib/get_num.h"

char *sigbus = "SIGBUS";
char *sigsegv = "SIGSEGV";

void handler(int s)
{
    char *sig;
    int saved_errno;
    char msg[100];
    switch (s) {
    case SIGSEGV:
        sig = sigsegv;
        break;
    case SIGBUS:
        sig = sigbus;
        break;
    default:
        _exit(1);
    }

    sprintf(msg, "%s signal handler invoked\n", sig);
    
    saved_errno = errno;
    write(STDOUT_FILENO, msg, strlen(msg));
    errno = saved_errno;
}

int main(int argc, char *argv[])
{
    char *p, *filename = "mmap.bin";
    int fd;
    long ps;
    int offset;

    if (argc < 2) {
        fprintf(stderr, "too few arguments.\n");
        exit(EXIT_FAILURE);
    }

    offset = getInt(argv[1], 0, "offset");

    fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror(filename);
        exit(1);
    }

    ps = sysconf(_SC_PAGESIZE);

    p = mmap(NULL, ps * 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    signal(SIGBUS, handler); 
    signal(SIGSEGV, handler);

    /*
        offset >= 4096 for SIGBUS
        offset >= 4096 * 4 for SIGSEGV (strange, why exectly 4 pages should be)
    */
    p[offset] = 1;

    return 0;
}