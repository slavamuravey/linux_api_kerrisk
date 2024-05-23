#define _GNU_SOURCE
#include <signal.h>
#include <syslog.h>
#include <sys/wait.h>
#include <sys/select.h>
#include "../lib/become_daemon.h"
#include "../lib/socket/inet_sockets.h"
#include "../lib/tlpi_hdr.h"
#include "../lib/tty/tty.h"

#define BUF_SIZE 256
#define MAX_SNAME 1000

static void grim_reaper(int sig)
{
    int saved_errno;

    saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0) {
        continue;
    }
        
    errno = saved_errno;
}

static void handle_request(int cfd)
{
    char slave_name[MAX_SNAME];
    int master_fd;
    pid_t child_pid;
    ssize_t num_read;
    fd_set in_fds;
    char buf[BUF_SIZE];
    
    child_pid = pty_fork(&master_fd, slave_name, MAX_SNAME, NULL, NULL);
    if (child_pid == -1) {
        perror("pty_fork");
        exit(1);
    }

    if (child_pid == 0) {
        char *login = "login";

        execlp(login, login, (char *) NULL);
        perror("execlp");
        exit(1);
    }
    
    for (;;) {
        int sd;
        FD_ZERO(&in_fds);
        FD_SET(cfd, &in_fds);
        FD_SET(master_fd, &in_fds);

        sd = select(cfd + 1, &in_fds, NULL, NULL, NULL);
        if (sd == -1) {          
            perror("select");
            exit(1);
        }

        if (FD_ISSET(cfd, &in_fds)) {
            num_read = read(cfd, buf, BUF_SIZE);
            if (num_read <= 0) {
                exit(EXIT_SUCCESS);
            }

            if (write(master_fd, buf, num_read) != num_read) {
                fprintf(stderr, "partial/failed write\n");
                exit(1);
            }
        }

        if (FD_ISSET(master_fd, &in_fds)) {
            num_read = read(master_fd, buf, BUF_SIZE);
            if (num_read <= 0) {
                exit(EXIT_SUCCESS);
            }

            if (write(cfd, buf, num_read) != num_read) {
                fprintf(stderr, "partial/failed write\n");
                exit(1);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    int lfd, cfd;               
    struct sigaction sa;

    if (argc != 2 || strcmp(argv[1], "--help") == 0) {
        usageErr("%s port\n", argv[0]);
    }

    if (becomeDaemon(0) == -1) {
        errExit("becomeDaemon");
    }
        
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = grim_reaper;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        syslog(LOG_ERR, "Error from sigaction(): %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    lfd = inetListen(argv[1], 10, NULL);
    if (lfd == -1) {
        syslog(LOG_ERR, "Could not create server socket (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (;;) {
        cfd = accept(lfd, NULL, NULL);  
        if (cfd == -1) {
            syslog(LOG_ERR, "Failure in accept(): %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        switch (fork()) {
        case -1:
            syslog(LOG_ERR, "Can't create child (%s)", strerror(errno));
            close(cfd);                 
            break;                      

        case 0:                         
            close(lfd);                 
            handle_request(cfd);
            _exit(EXIT_SUCCESS);

        default:                        
            close(cfd);                 
            break;                      
        }
    }
}