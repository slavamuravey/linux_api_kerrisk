#define _GNU_SOURCE
#include <stdio.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/times.h>

#define SOFT_LIMIT 10
#define HARD_LIMIT 15
#define STEP_PERCENT 25

void handler(int s)
{
    char msg[] = "soft limit reached\n";
    write(STDOUT_FILENO, "soft limit reached", sizeof(msg) - 1);
}

void load_cpu()
{
    struct tms tms;
    int sec_percent;
    int steps_percent = 0;
    while (1) {
        if (times(&tms) == -1) {
            perror("times");
            exit(1);
        }
        sec_percent = (tms.tms_stime + tms.tms_utime) * 100 / sysconf(_SC_CLK_TCK);

        if (sec_percent >= steps_percent + STEP_PERCENT) {
            steps_percent += STEP_PERCENT;
            printf("(PID %ld) cpu=%0.2f\n", (long) getpid(), steps_percent / 100.0);
        }
    }
}

int main(int argc, char *argv[])
{
    struct rlimit rlim;
    rlim.rlim_cur = SOFT_LIMIT;
    rlim.rlim_max = HARD_LIMIT;
    signal(SIGXCPU, handler);
    if (setrlimit(RLIMIT_CPU, &rlim) == -1) {
        perror("setrlimit");
        exit(1);
    }
    
    load_cpu();
    
    return 0;
}