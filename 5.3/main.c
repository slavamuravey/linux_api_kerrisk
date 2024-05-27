#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int fd;
    char *filename;
    size_t nbytes;
    int flags;
    int i;

    if (argc < 3) {
        fprintf(stderr, "too few arguments.\n");
        exit(EXIT_FAILURE);
    }

    filename = argv[1];
    nbytes = strtoul(argv[2], NULL, 10);

    flags = O_WRONLY;

    if (!argv[3]) {
        flags |= O_APPEND;
    }

    fd = open(filename, flags);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < nbytes; i ++) {
        char buf[1];
        
        if (lseek(fd, 0, SEEK_END) == -1) {
            perror("lseek");
            exit(EXIT_FAILURE);
        }
        
        buf[0] = 1;
        if (write(fd, buf, sizeof(buf)) < sizeof(buf)) {
            fprintf(stderr, "partial/failed write");
            exit(EXIT_FAILURE);
        }
    }
    
    return 0;
}
