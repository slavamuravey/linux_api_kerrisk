#define _XOPEN_SOURCE 600
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "tty.h"

#define MAX_SNAME 1000

int tty_set_cbreak(int fd, struct termios *prev_termios)
{
    struct termios t;

    if (tcgetattr(fd, &t) == -1) {
        return -1;
    }

    if (prev_termios != NULL) {
        *prev_termios = t;
    }
        
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_lflag |= ISIG;
    t.c_iflag &= ~ICRNL;
    t.c_cc[VMIN] = 1;                  
    t.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSAFLUSH, &t) == -1) {
        return -1;
    }

    return 0;
}

int tty_set_raw(int fd, struct termios *prev_termios)
{
    struct termios t;

    if (tcgetattr(fd, &t) == -1) {
        return -1;
    }

    if (prev_termios != NULL) {
        *prev_termios = t;
    }

    t.c_lflag &= ~(ICANON | ISIG | IEXTEN | ECHO);
    t.c_iflag &= ~(BRKINT | ICRNL | IGNBRK | IGNCR | INLCR | INPCK | ISTRIP | IXON | PARMRK);
    t.c_oflag &= ~OPOST;                
    t.c_cc[VMIN] = 1;                   
    t.c_cc[VTIME] = 0;                  

    if (tcsetattr(fd, TCSAFLUSH, &t) == -1) {
        return -1;
    }

    return 0;
}

int pty_master_open(char *slave_name, size_t sn_len)
{
    int master_fd, saved_errno;
    char *p;

    master_fd = posix_openpt(O_RDWR | O_NOCTTY);
    if (master_fd == -1) {
        return -1;
    }
        
    if (grantpt(master_fd) == -1) {
        saved_errno = errno;
        close(master_fd);
        errno = saved_errno;
        
        return -1;
    }

    if (unlockpt(master_fd) == -1) {
        saved_errno = errno;
        close(master_fd);
        errno = saved_errno;
        
        return -1;
    }

    p = ptsname(master_fd);
    if (p == NULL) {
        saved_errno = errno;
        close(master_fd);
        errno = saved_errno;
        
        return -1;
    }

    if (strlen(p) < sn_len) {
        strncpy(slave_name, p, sn_len);
    } else {
        close(master_fd);
        errno = EOVERFLOW;
        
        return -1;
    }

    return master_fd;
}

pid_t pty_fork(
    int *master_fd, 
    char *slave_name, 
    size_t sn_len,
    const struct termios *slave_termios, 
    const struct winsize *slave_ws
)
{
    int mfd, slave_fd, saved_errno;
    pid_t child_pid;
    char slname[MAX_SNAME];

    mfd = pty_master_open(slname, MAX_SNAME);
    if (mfd == -1) {
        return -1;
    }

    if (slave_name != NULL) {
        if (strlen(slname) < sn_len) {
            strncpy(slave_name, slname, sn_len);
        } else {
            close(mfd);
            errno = EOVERFLOW;
            
            return -1;
        }
    }

    child_pid = fork();
    if (child_pid == -1) {               
        saved_errno = errno;            
        close(mfd);                     
        errno = saved_errno;
        
        return -1;
    }

    if (child_pid != 0) {
        *master_fd = mfd;
        
        return child_pid;
    }

    if (setsid() == -1) {
        perror("setsid");
        exit(1);
    }
        
    close(mfd);                         

    slave_fd = open(slname, O_RDWR);
    if (slave_fd == -1) {
        perror("open");
        exit(1);
    }
        
    if (slave_termios != NULL) {
        if (tcsetattr(slave_fd, TCSANOW, slave_termios) == -1) {
            perror("tcsetattr");
            exit(1);
        }
    }

    if (slave_ws != NULL) {
        if (ioctl(slave_fd, TIOCSWINSZ, slave_ws) == -1) {
            perror("ioctl");
            exit(1);
        }
    }
        
    if (dup2(slave_fd, STDIN_FILENO) != STDIN_FILENO) {
        perror("dup2");
        exit(1);
    }
        
    if (dup2(slave_fd, STDOUT_FILENO) != STDOUT_FILENO) {
        perror("dup2");
        exit(1);
    }
        
    if (dup2(slave_fd, STDERR_FILENO) != STDERR_FILENO) {
        perror("dup2");
        exit(1);
    }

    if (slave_fd > STDERR_FILENO) {
        close(slave_fd);
    }

    return 0;                           
}
