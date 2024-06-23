#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "common.h"

long port_from_str(const char *str)
{
    char *endptr;
    long port;
    port = strtol(str, &endptr, 10);
    if (errno) {
        perror("strtol");
        exit(EXIT_FAILURE);
    }

    if (str == NULL || *str == '\0') {
        fprintf(stderr, "null or empty string.\n");
        exit(EXIT_FAILURE);
    }

    if (*endptr != '\0') {
        fprintf(stderr, "nonnumeric characters.\n");
        exit(EXIT_FAILURE);
    }

    if (port <= 0) {
        fprintf(stderr, "negative value is not allowed.\n");
        exit(EXIT_FAILURE);
    }

    return port;
}