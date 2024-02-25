#define _GNU_SOURCE
#define __USE_MISC
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <crypt.h>
#include <pwd.h>
#include <shadow.h>
#include <string.h>
#include <errno.h>
#include <grp.h>

extern char *optarg;
extern int optind, opterr;

int main(int argc, char *argv[])
{
    size_t argc_bufsize;
    char **saved_argv;
    char **argv_prog;
    int opt;
    char *username, *password, *encrypted;
    struct passwd *pwd;
    struct spwd *spwd;
    username = NULL;
    opterr = 0;
    argc_bufsize = argc * sizeof(char *);
    saved_argv = alloca(argc_bufsize);
    memcpy(saved_argv, argv, argc_bufsize);
    while ((opt = getopt(argc, saved_argv, "u:")) != -1) {
        if (opt == 'u' && optind == 3) {
            username = optarg;
            break;
        }
        if (opt == '?' && optind == 2) {
            fprintf(stderr, "usage: %s [-u user ] program-file arg1 arg2 ...\n", saved_argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "expected argument after options\n");
        exit(EXIT_FAILURE);
    }

    if (username == NULL) {
        argv_prog = argv + 1;
        username = "root";
    } else {
        argv_prog = argv + 3;
    }

    pwd = getpwnam(username);
    if (pwd == NULL) {
        fprintf(stderr, "couldn't get password record\n");
        exit(EXIT_FAILURE);
    }

    spwd = getspnam(username);
    if (spwd == NULL && errno == EACCES) {
        fprintf(stderr, "no permission to read shadow password file\n");
        exit(EXIT_FAILURE);
    }
    if (spwd != NULL) {
        pwd->pw_passwd = spwd->sp_pwdp;
    }

    password = getpass("Password: ");
    encrypted = crypt(password, pwd->pw_passwd);
    memset(password, 0, strlen(password));

    if (encrypted == NULL) {
        perror("crypt");
        exit(1);
    }

    if (strcmp(encrypted, pwd->pw_passwd)) {
        printf("incorrect password\n");
        exit(EXIT_FAILURE);
    }

    if (initgroups(username, pwd->pw_gid) == -1) {
        perror("initgroups");
        exit(1);
    }

    if (setgid(pwd->pw_gid) == -1) {
        perror("setgid");
        exit(1);
    }

    if (setuid(pwd->pw_uid) == -1) {
        perror("setuid");
        exit(1);
    }

    execvp(*argv_prog, argv_prog);
    perror(*argv_prog);
    exit(1);

    return 0;
}