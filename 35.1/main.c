#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "../lib/get_num.h"

extern char *optarg;
extern int optind;

int main(int argc, char *argv[])
{
    char **argv_prog;
    int opt;
    int prio;
    char *nvalue;

    if (argc == 1) {
        errno = 0;
        prio = getpriority(PRIO_PROCESS, getpid());
        if (prio == -1 && errno != 0) {
            perror("getpriority");
            exit(1);
        }

        printf("%d\n", prio);
        exit(0);
    }

    nvalue = NULL;
    while ((opt = getopt(argc, argv, "n:")) != -1) {
        switch (opt) {
        case 'n':
            nvalue = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s [-n] priority\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected argument after options\n");
        exit(EXIT_FAILURE);
    }

    if (nvalue == NULL) {
        argv_prog = argv + 1;
        prio = 10;
    } else {
        argv_prog = argv + 3;
        prio = getInt(nvalue, 0, "prio");
    }

    if (setpriority(PRIO_PROCESS, getpid(), prio) == -1) {
        perror("setpriority");
        exit(1);
    }

    execvp(*argv_prog, argv_prog);
    perror(*argv_prog);
    exit(1);

    return 0;
}