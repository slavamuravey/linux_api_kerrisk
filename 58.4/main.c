#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    struct termios tp;

    if (tcgetattr(STDIN_FILENO, &tp) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    printf("ICANON: %u\n", (tp.c_lflag & ICANON));
    printf("TIME: %u, MIN: %u\n", tp.c_cc[VTIME], tp.c_cc[VMIN]);
    
    return EXIT_SUCCESS;
}