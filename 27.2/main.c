#define _GNU_SOURCE
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <linux/limits.h>

#define BUFSIZE 4

extern char **environ;

struct argv {
    size_t len;
    size_t cap;
    char **ptr;
};

struct argv *argv_create(size_t cap)
{
    struct argv *argv = malloc(sizeof(struct argv));
    argv->cap = cap;
    argv->len = 0;
    argv->ptr = malloc(sizeof(char *));

    return argv;
}

void argv_append(struct argv *argv, const char *arg)
{
    argv->len++;

    if (argv->len > argv->cap) {
        argv->cap *= 2;
        argv->ptr = realloc(argv->ptr, sizeof(char *) * argv->cap);
    }

    argv->ptr[argv->len - 1] = (char *)arg;
}

int my_execlp(const char *file, const char *arg, ...)
{
    va_list vl;
    char *arg_cur;
    struct argv *argv;
    char *sep;
    char *prog_dir;
    char *path_env;
    char resolved_file[PATH_MAX];

    va_start(vl, arg);
    argv = argv_create(BUFSIZE);
    argv_append(argv, arg);
    while (1) {
        arg_cur = va_arg(vl, char *);
        argv_append(argv, arg_cur);
        if (arg_cur == NULL) {
            break;
        }
    }

    va_end(vl);

    path_env = getenv("PATH");
    sep = ":";  
    prog_dir = strtok(path_env, sep);  
    while(prog_dir != NULL) {
        sprintf(resolved_file, "%s/%s", prog_dir, file);
        execve(resolved_file, argv->ptr, environ);
        prog_dir = strtok(NULL, sep);   
    }
    
    return execve(file, argv->ptr, environ);
}

int main(int argc, char *argv[])
{
    putenv("ZXC=QWE");
    my_execlp("env", "env", NULL);
    perror("execlp");
    exit(1);
}
