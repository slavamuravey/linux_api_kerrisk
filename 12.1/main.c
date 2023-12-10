#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include <stdarg.h>

void str_split_words(const char *str, ...)
{
    va_list vl;
    char *word;
    const char *p, *word_start;
    int isword = 0;

    va_start(vl, str);    
    
    for (p = str; *p; p++) {
        if (isspace(*p) || p - str == strlen(str)) {
            if (isword) {
                size_t word_len = p - word_start;
                isword = 0;
                word = va_arg(vl, char *);
                if (!word) {
                    break;
                }
                strncpy(word, word_start, word_len);
                word[word_len] = '\0';
            }
        } else {
            if (!isword) {
                isword = 1;
                word_start = p;
            }
        }
    }

    va_end(vl);
}

uid_t user_id_from_name(const char *name)
{
    struct passwd *pwd;
    uid_t u;
    char *endptr;
    if (name == NULL || *name == '\0') {
        return -1;
    }

    u = strtol(name, &endptr, 10);
    if (*endptr == '\0') {
        return u;
    }
    
    pwd = getpwnam(name);
    if (pwd == NULL) {
        return -1;
    }
    
    return pwd->pw_uid;
}

int isnumber(const char *str)
{
    const char *p;
    for (p = str; *p; p++) {
        if (!isdigit(*p)) {
            return 0;
        }
    }

    return 1;
}

int main(int argc, char *argv[])
{
    char *username;
    DIR *proc_dir;
    struct dirent *dent;
    uid_t uid, euid;
    FILE *proc_status_stream;

    if (argc < 2) {
        fprintf(stderr, "too few arguments.\n");
        exit(EXIT_FAILURE);
    }

    username = argv[1];

    uid = user_id_from_name(username);
    if (uid == -1) {
        fprintf(stderr, "no such user (%s)\n", username);
        exit(EXIT_FAILURE);
    }

    proc_dir = opendir("/proc");
    if (!proc_dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    while (1) {
        ssize_t nread;
        char proc_status_filename[300];
        char *line = NULL;
        size_t len = 0;
        char *uid_field = "Uid:";
        char *name_field = "Name:";
        char field_name[100] = "";
        char ruid_str[100] = "";
        char euid_str[100] = "";
        char proc_name[100] = "";

        errno = 0;
        
        dent = readdir(proc_dir);
        if (!dent) {
            if (errno) {
                perror("readdir");
                exit(EXIT_FAILURE);
            }

            break;
        }

        if (!isnumber(dent->d_name)) {
            continue;
        }

        if (sprintf(proc_status_filename, "/proc/%s/status", dent->d_name) < 0) {
            perror("sprintf");
            exit(EXIT_FAILURE);
        }

        proc_status_stream = fopen(proc_status_filename, "r");
        if (!proc_status_stream) {
            continue;
        }

        while ((nread = getline(&line, &len, proc_status_stream)) != -1) {
            if (!strncmp(uid_field, line, strlen(uid_field))) {
                str_split_words(line, field_name, ruid_str, euid_str, NULL);
            }
            
            if (!strncmp(name_field, line, strlen(name_field))) {
                str_split_words(line, field_name, proc_name, NULL);
            }
        }

        free(line);
        fclose(proc_status_stream);

        euid = strtol(euid_str, NULL, 10);
        if (errno) {
            perror("strtol");
            exit(EXIT_FAILURE);
        }

        if (uid == euid) {
            printf("pid = %s, cmd = %s\n", dent->d_name, proc_name);
        }
    }

    return 0;
}
