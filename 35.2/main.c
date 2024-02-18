#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>
#include "../lib/get_num.h"

int main(int argc, char *argv[])
{
    char **argv_prog;
    char *policy_arg, *prio_arg;
    int policy, prio;
    struct sched_param sp;

    if (argc < 4) {
        fprintf(stderr, "too few arguments.\n");
        exit(EXIT_FAILURE);
    }

    policy_arg = argv[1];
    prio_arg = argv[2];
    prio = getInt(prio_arg, 0, "prio");
    if (!strcmp(policy_arg, "r")) {
        policy = SCHED_RR;
    } else if (!strcmp(policy_arg, "f")) {
        policy = SCHED_FIFO;
    } else {
        fprintf(stderr, "policy argument should be \"r\" or \"f\".\n");
        exit(EXIT_FAILURE);
    }

    sp.sched_priority = prio;

    if (sched_setscheduler(getpid(), policy, &sp) == -1) {
        perror("sched_setscheduler");
        exit(EXIT_FAILURE);
    }

    if (seteuid(getuid()) == -1) {
        perror("seteuid");
        exit(EXIT_FAILURE);
    }

    argv_prog = argv + 3;

    execvp(*argv_prog, argv_prog);
    perror(*argv_prog);
    exit(1);

    return 0;
}