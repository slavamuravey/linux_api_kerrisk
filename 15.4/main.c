#define _GNU_SOURCE
#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int my_eaccess(const char *name, int type)
{
    int res;
    int saved_errno;

    uid_t ruid, euid, suid;
    gid_t rgid, egid, sgid;

    getresuid(&ruid, &euid, &suid);
    getresgid(&rgid, &egid, &sgid);

    if (setresuid(euid, -1, ruid) == -1) {
        return -1;
    }
    if (setresgid(egid, -1, rgid) == -1) {
        return -1;
    }

    saved_errno = errno;
    res = access(name, type);

    if (setresuid(ruid, -1, suid) == -1) {
        errno = saved_errno;
        return -1;
    }
    if (setresgid(rgid, -1, sgid) == -1) {
        errno = saved_errno;
        return -1;
    }

    return res;
}

int Access(const char *name, int type)
{
    return my_eaccess(name, type);
}

int main(int argc, char *argv[])
{
    char *filename;
    
    if (argc < 2) {
        fprintf(stderr, "too few arguments.\n");
        exit(EXIT_FAILURE);
    }

    filename = argv[1];
    
    if (Access(filename, F_OK) != -1) {
       printf("%s is exists\n", filename); 
    }

    if (Access(filename, R_OK) != -1) {
       printf("%s is readable\n", filename); 
    }

    if (Access(filename, W_OK) != -1) {
       printf("%s is writable\n", filename); 
    }

    if (Access(filename, X_OK) != -1) {
       printf("%s is executable\n", filename); 
    }

    return 0;
}
