#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern int optind;

int main(int argc, char **argv)
{
    int c;
    FILE *stream;
    int opt;
    char *filename;
    char *mode = "w";
    
    while ((opt = getopt(argc, argv, "ar")) != -1) {
        switch (opt) {
        case 'a':
            mode = "a";
            break;
        default:
            fprintf(stderr, "Usage: %s [-ar] filename\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected argument after options\n");
        exit(EXIT_FAILURE);
    }

    filename = argv[optind];

    if (!(stream = fopen(filename, mode))) {
        perror(filename);
        exit(EXIT_FAILURE);
    }
    
    while ((c = getchar()) != EOF) {
        putchar(c);
        fputc(c, stream);
    }

    return 0;
}
