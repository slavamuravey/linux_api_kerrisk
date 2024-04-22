#define _XOPEN_SOURCE
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define PASS_MAX 8192

static char pass_buf[PASS_MAX];

char *my_getpass(const char *prompt)
{
    int tty_fd;
    struct termios tp, old_tp;
    ssize_t nbytes;
    char *ptr;
    size_t prompt_len;
    
    tty_fd = open("/dev/tty", O_RDWR);
    if (tty_fd == -1) {
        return NULL;
    }

    if (tcgetattr(tty_fd, &old_tp) == -1) {
        close(tty_fd);
        
        return NULL;
    }

    prompt_len = strlen(prompt);
    if (write(tty_fd, prompt, prompt_len) < prompt_len) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    tp = old_tp;
    
    tp.c_lflag &= ~ECHO;
    tp.c_lflag |= ECHONL;
    if (tcsetattr(tty_fd, TCSAFLUSH, &tp) == -1) {
        close(tty_fd);
        
        return NULL;
    }

    nbytes = read(tty_fd, pass_buf, sizeof(pass_buf));
    if (nbytes == -1) {
        if (errno == EINTR) {
            return NULL;
        }
        
        perror("read");
        exit(EXIT_FAILURE);
    }

    ptr = pass_buf + nbytes - 1;
    if (*ptr == '\n') {
        *ptr = '\0';
    }

    if (tcsetattr(tty_fd, TCSAFLUSH, &old_tp) == -1) {
        close(tty_fd);
        
        return NULL;
    }
    
    if (close(tty_fd) == -1) {
        return NULL;
    }

    return pass_buf; 
}

int main(int argc, char *argv[])
{
    char *pass;
    pass = my_getpass("Password prompt: ");

    printf("Password is: %s\n", pass);

    return 0;
}