#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void write_data(int fd)
{
    char data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    size_t data_size = sizeof(data);
    ssize_t bytes_count = write(fd, data, data_size);
    
    if (bytes_count == -1) {
        perror("write");
        exit(EXIT_FAILURE);
    }
    
    if (bytes_count != data_size) {
        fprintf(stderr, "Partial/failed write");
        exit(EXIT_FAILURE);
    }
}

void write_hole(int fd)
{
    if (lseek(fd, 1000000, SEEK_END) == -1) {
        perror("lseek");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    int fd = open("hole.dat", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    write_hole(fd);
    write_data(fd);
    write_hole(fd);
    write_data(fd);
    write_hole(fd);
    write_data(fd);

    return 0;
}
