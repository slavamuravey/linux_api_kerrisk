#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int i;
    char *str;
    
    if (argc < 2) {
        fprintf(stderr, "too few arguments.\n");
        exit(EXIT_FAILURE);
    }

    str = argv[1];

    for (i = 0; i < 10000; i++) {
        printf("%s%d\n", str, i);
        usleep(10000);
    }
    
    return 0;
}
