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
        exit(EXIT_FAILURE);
        perror("tcgetattr");
    }

    printf("ICANON: %s\n", tp.c_lflag && ICANON ? "true" : "false");
    printf("TIME: %u, MIN: %u\n", tp.c_cc[VTIME], tp.c_cc[VMIN]);
    
    return EXIT_SUCCESS;
}