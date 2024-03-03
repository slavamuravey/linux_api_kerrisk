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
    int src_fd, dst_fd;
    char *src_filename, *dst_filename;
    char *src_ptr, *dst_ptr;
    struct stat sb;
    
    if (argc < 3) {
        fprintf(stderr, "too few arguments.\n");
        exit(EXIT_FAILURE);
    }

    src_filename = argv[1];
    dst_filename = argv[2];

    src_fd = open(src_filename, O_RDONLY);
    if (src_fd == -1) {
        perror(src_filename);
        exit(1);
    }

    dst_fd = open(dst_filename, O_RDWR | O_TRUNC | O_CREAT, 0666);
    if (dst_fd == -1) {
        perror(dst_filename);
        exit(1);
    }

    if (fstat(src_fd, &sb) == -1) {
        perror("fstat");
        exit(1);
    }

    if (ftruncate(dst_fd, sb.st_size) == -1) {
        perror("ftruncate");
        exit(1);
    }

    src_ptr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
    if (src_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    dst_ptr = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, dst_fd, 0);
    if (dst_ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    memcpy(dst_ptr, src_ptr, sb.st_size);

    if (msync(dst_ptr, sb.st_size, MS_SYNC) == -1) {
        perror("msync");
        exit(1);
    }

    if (munmap(src_ptr, sb.st_size) == -1) {
        perror("munmap");
        exit(1);
    }

    if (munmap(dst_ptr, sb.st_size) == -1) {
        perror("munmap");
        exit(1);
    }

    return 0;
}