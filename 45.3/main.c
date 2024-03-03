#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    char *p, *p1, *p2, *p3, *filename = "mmap.bin";
    int fd;
    struct stat sb;
    long ps;
    
    fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror(filename);
        exit(1);
    }

    if (fstat(fd, &sb) == -1) {
        perror("fstat");
        exit(1);
    }

    p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    ps = sysconf(_SC_PAGESIZE);

    p1 = mmap(p, ps, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, ps + ps);
    if (p1 == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    p2 = mmap(p + ps, ps, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, ps);
    if (p2 == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    p3 = mmap(p + ps + ps, ps, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, 0);
    if (p3 == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    p[0] = 1;
    p[ps] = 2;
    p[ps + ps] = 3;

    return 0;
}