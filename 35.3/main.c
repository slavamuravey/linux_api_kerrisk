#define _GNU_SOURCE
#include <sys/times.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <sys/resource.h>

#define STEP_PERCENT 25

void load_cpu(const char *msg)
{
    struct tms tms;
    int sec_percent;
    int steps_percent = 0;
    int secs_percent = 0;
    while (1) {
        if (times(&tms) == -1) {
            perror("times");
            exit(1);
        }
        sec_percent = (tms.tms_stime + tms.tms_utime) * 100 / sysconf(_SC_CLK_TCK);

        if (sec_percent >= steps_percent + STEP_PERCENT) {
            steps_percent += STEP_PERCENT;
            printf("%s (PID %ld) cpu=%0.2f\n", msg, (long) getpid(), steps_percent / 100.0);
        }

        if (sec_percent > 300) {
            return;
        }

        if (steps_percent >= secs_percent + 100) {
            secs_percent = steps_percent;
            sched_yield();
        }
    }
}

int main(int argc, char *argv[])
{
    struct sched_param sp;
    struct rlimit rlim;
    pid_t pid;
    int prio;
    cpu_set_t set;

    CPU_ZERO(&set);
    CPU_SET(1, &set);

    if (sched_setaffinity(getpid(), sizeof(set), &set) == -1) {
        perror("sched_setaffinity");
        exit(EXIT_FAILURE);
    }

    rlim.rlim_cur = rlim.rlim_max = 50;
    if (setrlimit(RLIMIT_CPU, &rlim) == -1) {
        perror("setrlimit");
        exit(EXIT_FAILURE);
    }

    prio = sched_get_priority_min(SCHED_FIFO);
    sp.sched_priority = prio;

    if (sched_setscheduler(0, SCHED_FIFO, &sp) == -1) {
        perror("sched_setscheduler");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        load_cpu("child");
    } else {
        load_cpu("parent");
    }

    return 0;
}
