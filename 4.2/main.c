#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

off_t Lseek(int fd, off_t offset, int whence)
{
    off_t off_res;
    if ((off_res = lseek(fd, offset, whence)) == -1) {
        perror("lseek");
        exit(EXIT_FAILURE);
    }

    return off_res;
}

int main(int argc, char **argv)
{
    char *src_filename, *dst_filename;
    int src_fd, dst_fd;

    if (argc < 3) {
        fprintf(stderr, "Too few arguments.\n");
        exit(EXIT_FAILURE);
    }

    src_filename = argv[1];
    dst_filename = argv[2];

    src_fd = open(src_filename, O_RDONLY);
    if (src_fd == -1) {
        perror(src_filename);
        exit(EXIT_FAILURE);
    }

    dst_fd = open(dst_filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (dst_fd == -1) {
        perror(dst_filename);
        exit(EXIT_FAILURE);
    }

    while (1) {
        off_t off_cur, off_data, off_hole, off_end;
        size_t data_area;

        off_cur = Lseek(src_fd, 0, SEEK_CUR);
        off_end = Lseek(src_fd, 0, SEEK_END);
        
        if (off_cur == off_end) {
            break;
        }

        Lseek(src_fd, off_cur, SEEK_SET);
        off_hole = Lseek(src_fd, off_cur, SEEK_HOLE);
        Lseek(src_fd, off_cur, SEEK_SET);

        if (off_cur == off_hole) {
            off_data = Lseek(src_fd, off_cur, SEEK_DATA);
            Lseek(dst_fd, off_data, SEEK_SET);

            continue;
        }

        data_area = off_hole - off_cur;

        while (data_area > 0) {
            ssize_t r_bytes_count, w_bytes_count;
            char buf[4096];
            size_t buf_size = MIN(sizeof(buf), data_area);
            data_area -= buf_size;

            r_bytes_count = read(src_fd, buf, buf_size);
            if (r_bytes_count == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }

            w_bytes_count = write(dst_fd, buf, r_bytes_count);
            if (w_bytes_count == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            
            if (w_bytes_count != r_bytes_count) {
                fprintf(stderr, "Partial/failed write");
                exit(EXIT_FAILURE);
            }
        }
    }

    return 0;
}
