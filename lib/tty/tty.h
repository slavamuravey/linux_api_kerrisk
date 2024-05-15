#ifndef TTY_H
#define TTY_H

#include <termios.h>
#include <sys/types.h>
#include <sys/ioctl.h>

int tty_set_cbreak(int fd, struct termios *prev_termios);

int tty_set_raw(int fd, struct termios *prev_termios);

int pty_master_open(char *slave_name, size_t sn_len);

pid_t pty_fork(
    int *master_fd, 
    char *slave_name, 
    size_t sn_len, 
    const struct termios *slave_termios, 
    const struct winsize *slave_ws
);

#endif