#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <linux/limits.h>
#include <string.h>

static char tty[PATH_MAX];
static char *tty_dirs[2] = {"/dev", "/dev/pts"};

char *my_ttyname(int fd)
{
    DIR *tty_dir_stream;
    struct stat sb, tty_sb;
    struct dirent *dent;
    int i;
    
    if (!isatty(fd)) {
        return NULL;
    }

    if (fstat(fd, &sb) == -1) {
        return NULL;
    }

    if (!S_ISCHR(sb.st_mode)) {
        return NULL;
    }

    for (i = 0; i < sizeof(tty_dirs) / sizeof(*tty_dirs); i++) {
        char *tty_dir = tty_dirs[i];
        tty_dir_stream = opendir(tty_dirs[i]);
        if (!tty_dir_stream) {
            perror("opendir");
            exit(EXIT_FAILURE);
        }

        while (1) {
            errno = 0;
            dent = readdir(tty_dir_stream);
            if (!dent) {
                if (errno) {
                    perror("readdir");
                    exit(EXIT_FAILURE);
                }

                break;
            }

            memset(tty, 0, sizeof(tty));
            if (sprintf(tty, "%s/%s", tty_dir, dent->d_name) < 0) {
                perror("sprintf");
                exit(EXIT_FAILURE);
            }

            if (lstat(tty, &tty_sb) == -1) {
                continue;
            }

            if (sb.st_rdev == tty_sb.st_rdev && S_ISCHR(tty_sb.st_mode)) {
                closedir(tty_dir_stream);
                
                return tty;
            }
        }

        closedir(tty_dir_stream);
    }

    return NULL;
}

void test_ttyname(int fd)
{
    char *tty;
    tty = my_ttyname(fd);
    if (!tty) {
        perror("ttyname");
    } else {
        printf("%s\n", tty);
    }
}

int main(int argc, char *argv[])
{
    int fd;
    fd = open("/dev/null", O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    
    test_ttyname(STDERR_FILENO);
    test_ttyname(fd);
    test_ttyname(100500);

    close(fd);

    return 0;
}