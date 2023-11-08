#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/uio.h>
#include <bits/xopen_lim.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

ssize_t my_readv(int fd, const struct iovec *iov, int iovcnt)
{
    int i;
    size_t requested_bytes = 0;
    ssize_t read_bytes, tmp_bytes;
    char *buf, *tmp_buf;

    if (iovcnt < 0 || iovcnt > IOV_MAX) {
        errno = EINVAL;

        return -1;
    }

    for (i = 0; i < iovcnt; i++) {
        requested_bytes += iov[i].iov_len;
    }

    buf = malloc(requested_bytes);
    if (buf == NULL) {
        return -1;
    }
    
    tmp_buf = buf;
    read_bytes = read(fd, buf, requested_bytes);
    tmp_bytes = read_bytes;

    for (i = 0; i < iovcnt; i++) {
        size_t len;
        if (tmp_bytes < 0) {
            break;
        }
        len = iov[i].iov_len;
        if (tmp_bytes < len) {
            len = tmp_bytes;
        }
        memcpy(iov[i].iov_base, tmp_buf, len);
        tmp_buf += len;
        tmp_bytes -= len;
    }

    free(buf);

    return read_bytes;
}

ssize_t my_writev(int fd, const struct iovec *iov, int iovcnt)
{
    int i;
    size_t requested_bytes = 0;
    ssize_t write_bytes;
    char *buf, *tmp_buf;
    if (iovcnt < 0 || iovcnt > IOV_MAX) {
        errno = EINVAL;

        return -1;
    }

    for (i = 0; i < iovcnt; i++) {
        requested_bytes += iov[i].iov_len;
    }
    
    buf = malloc(requested_bytes);
    if (buf == NULL) {
        return -1;
    }

    tmp_buf = buf;

    for (i = 0; i < iovcnt; i++) {
        size_t len = iov[i].iov_len;
        memcpy(tmp_buf, iov[i].iov_base, len);
        tmp_buf += len;
    }

    write_bytes = write(fd, buf, requested_bytes);

    free(buf);

    return write_bytes;
}

int main()
{
    int data1 = 0;
    char data2 = 0;
    int data3 = 0;

    size_t total_len = 0;
    struct iovec iov[3];

    ssize_t read_bytes;
    char *readv_filename = "readv.dat";
    char *writev_filename = "writev.dat";
    int fd;
    
    fd = open(readv_filename, O_RDONLY);
    if (fd == -1) {
        perror(readv_filename);
        exit(1);
    }
    
    iov[0].iov_base = &data1;
    iov[0].iov_len = sizeof(data1);
    total_len += iov[0].iov_len;

    iov[1].iov_base = &data2;
    iov[1].iov_len = sizeof(data2);
    total_len += iov[1].iov_len;

    iov[2].iov_base = &data3;
    iov[2].iov_len = sizeof(data3);
    total_len += iov[2].iov_len;

    read_bytes = my_readv(fd, iov, 3);

    printf("total_len: %ld, read_bytes: %ld, "
    "data1: %ld, data2: %ld, data3: %ld\n", (long)total_len, (long)read_bytes, (long)data1, (long)data2, (long)data3);

    if (read_bytes < total_len) {
        printf("Read fewer bytes than requested\n");
    }

    fd = open(writev_filename, O_CREAT | O_TRUNC | O_WRONLY, 0666);
    if (fd == -1) {
        perror(writev_filename);
        exit(1);
    }

    my_writev(fd, iov, 3);

    return 0;
}
