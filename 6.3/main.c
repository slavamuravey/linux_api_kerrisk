#define _XOPEN_SOURCE
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

extern char **environ;

char *fake_environ[] = {"QWE=1", "QWE=2", "ZXC=asd"};

int my_setenv(const char *name, const char *value, int overwrite)
{
    char *env_str;
    
    if (name == NULL || name[0] == '\0' || strchr(name, '=') != NULL) {
        errno = EINVAL;
        
        return -1;
    }

    if (!overwrite && getenv(name)) {
        return 0;
    }

    env_str = malloc(strlen(name) + strlen(value) + 2);

    if (env_str == NULL) {
        return -1;
    }

    sprintf(env_str, "%s=%s", name, value);

    return !putenv((char *)env_str) ? 0 : -1;
}

int my_unsetenv(const char *name)
{
    size_t name_len;
    char **env;
    
    if (name == NULL || name[0] == '\0' || strchr(name, '=') != NULL) {
        errno = EINVAL;
        
        return -1;
    }

    name_len = strlen(name);
    env = environ;
    while (*env) {
        if (strncmp(*env, name, name_len) == 0 && (*env)[name_len] == '=') {
            char **env_rest;

            for (env_rest = env; *env_rest; env_rest++) {
                *env_rest = *(env_rest + 1);
            }
            
            continue;
        }
        env++;
    }
    
    return 0;
}

int main(int argc, char *argv[])
{
    char **env;

    printf("pid: %d\n", getpid());

    environ = fake_environ;

    printf("environ: %p\n", (void *)environ);

    my_unsetenv("QWE");
    my_setenv("ZXC", "zxc", 1);

    for (env = environ; *env; env++) {
        printf("%s\n", *env);
    }

    printf("environ: %p\n", (void *)environ);

    return 0;
}
