#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <ftw.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

static int inotify_fd;

static void Nftw(const char *dir, __nftw_func_t func, int descriptors, int flag)
{
    if (nftw(dir, func, descriptors, flag) == -1) {
        perror("nftw");
        exit(EXIT_FAILURE);
    }
}

static void print_inotify_event(struct inotify_event *e)
{
    printf("\twd =%2d; ", e->wd);
    if (e->cookie > 0) {
        printf("cookie =%4d; ", e->cookie);
    }

    printf("mask = ");
    if (e->mask & IN_ACCESS) {
        printf("IN_ACCESS ");
    }
    if (e->mask & IN_ATTRIB) {
        printf("IN_ATTRIB ");
    }
    if (e->mask & IN_CLOSE_NOWRITE) {
        printf("IN_CLOSE_NOWRITE ");
    }
    if (e->mask & IN_CLOSE_WRITE) {
        printf("IN_CLOSE_WRITE ");
    }
    if (e->mask & IN_CREATE) {
        printf("IN_CREATE ");
    }
    if (e->mask & IN_DELETE) {
        printf("IN_DELETE ");
    }
    if (e->mask & IN_DELETE_SELF) {
        printf("IN_DELETE_SELF ");
    }
    if (e->mask & IN_IGNORED) {
        printf("IN_IGNORED ");
    }
    if (e->mask & IN_ISDIR) {
        printf("IN_ISDIR ");
    }
    if (e->mask & IN_MODIFY) {
        printf("IN_MODIFY ");
    }
    if (e->mask & IN_MOVE_SELF) {
        printf("IN_MOVE_SELF ");
    }
    if (e->mask & IN_MOVED_FROM) {
        printf("IN_MOVED_FROM ");
    }
    if (e->mask & IN_MOVED_TO) {
        printf("IN_MOVED_TO ");
    }
    if (e->mask & IN_OPEN) {
        printf("IN_OPEN ");
    }
    if (e->mask & IN_Q_OVERFLOW) {
        printf("IN_Q_OVERFLOW ");
    }
    if (e->mask & IN_UNMOUNT) {
        printf("IN_UNMOUNT ");
    }

    printf("\n");
    
    if (e->len > 0) {
        printf("\t\tname = %s\n", e->name);
    }
}

static int entity_handler(const char *pathname, const struct stat *sbuf, int type, struct FTW *ftwb)
{
    int wd;
    
    if (type == FTW_NS) {
        return -1;
    }
    
    wd = inotify_add_watch(inotify_fd, pathname, IN_MOVED_FROM | IN_MOVED_TO | IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVE_SELF);
    if (wd == -1) {
        perror("inotify_add_watch");
        exit(EXIT_FAILURE);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    ssize_t num_read;
    char buf[BUF_LEN];
    char *p;
    struct inotify_event *e;
    char *dirname;

    if (argc < 2) {
        fprintf(stderr, "too few arguments.\n");
        exit(EXIT_FAILURE);
    }

    dirname = argv[1];
    
    inotify_fd = inotify_init();
    if (inotify_fd == -1) {
        perror("inotify_init");
        exit(EXIT_FAILURE);
    }

    Nftw(dirname, entity_handler, 10, 0);

    while (1) {
        num_read = read(inotify_fd, buf, BUF_LEN);

        if (num_read == 0) {
            fprintf(stderr, "zero bytes returned by read\n");
            exit(EXIT_FAILURE);
        }

        if (num_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        for (p = buf; p < buf + num_read;) {
            e = (struct inotify_event *)p;
            p += sizeof(struct inotify_event) + e->len;

            if (e->mask & IN_CREATE) {
                Nftw(dirname, entity_handler, 10, 0);
            }

            print_inotify_event(e);
        }
    }

    return 0;
}