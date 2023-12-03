#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE 500
#include <sys/types.h>
#include <grp.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>

int my_initgroups(const char *user, gid_t group)
{
    size_t gidsetsize = 1;
    gid_t grouplist[NGROUPS_MAX + 1];
    struct group *grp;

    grouplist[0] = group;

    setgrent();

    while ((grp = getgrent()) != NULL) {
        char **memp;
        for (memp = grp->gr_mem; *memp; memp++) {
            if (!strcmp(user, *memp)) {
                grouplist[gidsetsize] = grp->gr_gid;
                gidsetsize++;
            }
        }
    }

    endgrent();

    return setgroups(gidsetsize, grouplist);
}

int main(int argc, char *argv[])
{
    gid_t grouplist[NGROUPS_MAX + 1];
    size_t gidsetsize;
    int i;
    char *username;

    if (argc < 2) {
        fprintf(stderr, "too few arguments.\n");
        exit(EXIT_FAILURE);
    }

    username = argv[1];

    if (my_initgroups(username, 0) == -1) {
        perror("my_initgroups");
        exit(EXIT_FAILURE);
    }
    
    gidsetsize = getgroups(0, NULL);
    getgroups(gidsetsize, grouplist);

    for (i = 0; i < gidsetsize; i++) {
        printf("%d\n", grouplist[i]);
    }

    return 0;
}