#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct passwd *my_getpwnam(const char *name)
{
    struct passwd *pwd;

    setpwent();

    while ((pwd = getpwent()) != NULL) {
        if (!strcmp(name, pwd->pw_name)) {
            endpwent();
            return pwd;
        }
    }

    endpwent();
    
    return NULL;
}

int main(int argc, char *argv[])
{
    struct passwd *pwd;
    char *username;
    uid_t uid = geteuid();
    pwd = getpwuid(uid);
    if (!pwd) {
        fprintf(stderr, "can't find user info for uid %d\n", uid);
        exit(1);
    }

    username = pwd->pw_name;
    pwd = my_getpwnam(username);
    if (!pwd) {
        fprintf(stderr, "can't find user info for username \"%s\"\n", username);
        exit(1);
    }

    puts("user info:");
    printf("pw_name = %s\n", pwd->pw_name);
    printf("pw_passwd = %s\n", pwd->pw_passwd);
    printf("pw_uid = %ld\n", (long)pwd->pw_uid);
    printf("pw_gid = %ld\n", (long)pwd->pw_gid);
    printf("pw_gecos = %s\n", pwd->pw_gecos);
    printf("pw_dir = %s\n", pwd->pw_dir);
    printf("pw_shell = %s\n", pwd->pw_shell);

    return 0;
}