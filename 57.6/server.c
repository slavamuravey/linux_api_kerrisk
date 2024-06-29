#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>
#include "dynamic_array.h"
#include "common.h"

#define BACKLOG 5
#define CONS_CAP 2

struct server {
    int sock;
    struct dynamic_array cons;
};

int server_listen(struct server *server, const char *port)
{
    struct sockaddr_in addr;
    int opt;

    server->sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server->sock == -1) {
        return -1;
    }

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_from_str(port));

    opt = 1;
    if (setsockopt(server->sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        return -1;
    }
    
    if (bind(server->sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
        return -1;
    }

    if (listen(server->sock, BACKLOG) == -1) {
        return -1;
    }

    return 0;
}

int server_accept(struct server *server)
{
    struct sockaddr_in urg_addr, con_addr;
    struct urg_handshake_req req;
    struct urg_handshake_res res;
    ssize_t nbytes;
    socklen_t con_addr_len;
    struct connection con;

    con_addr_len = sizeof(con_addr);
    con.sock = accept(server->sock, (struct sockaddr *)&con_addr, &con_addr_len);
    if (con.sock == -1) {
        return -1;
    }

    nbytes = read(con.sock, &req, sizeof(req));
    if (nbytes == -1) {
        shutdown(con.sock, SHUT_RDWR);
        close(con.sock);
        
        return -1;
    }

    con.urg_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (con.urg_sock == -1) {
        shutdown(con.sock, SHUT_RDWR);
        close(con.sock);
        
        return -1;
    }

    memset(&urg_addr, 0, sizeof(struct sockaddr_in));
    urg_addr.sin_family = con_addr.sin_family;
    urg_addr.sin_addr.s_addr = INADDR_ANY;
    urg_addr.sin_port = req.port;

    if (connect(con.urg_sock, (struct sockaddr *)&urg_addr, sizeof(urg_addr)) == -1) {
        shutdown(con.sock, SHUT_RDWR);
        close(con.sock);
        
        return -1;
    }

    memset(&res, 0, sizeof(res));
    strcpy(res.con_id, req.con_id);
    if (write(con.urg_sock, &res, sizeof(res)) != sizeof(res)) {
        shutdown(con.sock, SHUT_RDWR);
        shutdown(con.urg_sock, SHUT_RDWR);
        close(con.sock);
        close(con.urg_sock);
        errno = EIO;
        
        return -1;
    }

    dynamic_array_append(&server->cons, &con);
    
    printf("connection established\n");

    printf(
        "[connections count: %ld, sock fd: %d, urg sock fd: %d, token: %s]\n", 
        server->cons.len,
        con.sock,
        con.urg_sock,
        req.con_id
    );

    return 0;
}

void server_run(struct server *server)
{
    fd_set in_fds;
    int fd_max;
    int sd;
    int i;
    
    while (1) {
        FD_ZERO(&in_fds);
        FD_SET(server->sock, &in_fds);
        fd_max = server->sock;
        for (i = 0; i < server->cons.len; i++) {
            struct connection *cons;
            struct connection con;
            cons = server->cons.ptr;
            con = cons[i];
            FD_SET(con.sock, &in_fds);
            FD_SET(con.urg_sock, &in_fds);
            
            if (con.sock > fd_max) {
                fd_max = con.sock;
            }
            if (con.urg_sock > fd_max) {
                fd_max = con.urg_sock;
            }
        }
        
        sd = select(fd_max + 1, &in_fds, NULL, NULL, NULL);
        if (sd == -1) {
            if (errno != EINTR) {
                perror("select");
                exit(1);
            }
            continue;
        }

        if (FD_ISSET(server->sock, &in_fds)) {
            if (server_accept(server) == -1) {
                perror("server_accept");
                continue;
            }
        }
        
        for (i = 0; i < server->cons.len; i++) {
            ssize_t nbytes;
            char buf[BUFSIZE];
            struct connection *cons;
            struct connection con;
            cons = server->cons.ptr;
            con = cons[i];
            
            if (FD_ISSET(con.sock, &in_fds)) {
                nbytes = read(con.sock, buf, sizeof(buf));
                if (nbytes == -1) {
                    if (errno != EINTR) {
                        perror("read");
                    }

                    continue;
                }

                if (nbytes == 0) {
                    shutdown(con.sock, SHUT_RDWR);
                    shutdown(con.urg_sock, SHUT_RDWR);
                    close(con.sock);
                    close(con.urg_sock);
                    dynamic_array_remove(&server->cons, i);
                    
                    printf("connection closed\n");
                    printf("[connections count: %ld]\n", server->cons.len);
                    
                    continue;
                }

                printf("regular data received\n");

                if (write(STDOUT_FILENO, buf, nbytes) != nbytes) {
                    fprintf(stderr, "partial/failed write\n");
                    
                    continue;
                }

                if (write(con.sock, buf, nbytes) != nbytes) {
                    fprintf(stderr, "partial/failed write\n");
                    
                    continue;
                }
            }

            if (FD_ISSET(con.urg_sock, &in_fds)) {
                nbytes = read(con.urg_sock, buf, sizeof(buf));
                if (nbytes == -1) {
                    if (errno != EINTR) {
                        perror("read");
                    }

                    continue;
                }

                if (nbytes == 0) {
                    dynamic_array_remove(&server->cons, i);
                    shutdown(con.sock, SHUT_RDWR);
                    shutdown(con.urg_sock, SHUT_RDWR);
                    close(con.sock);
                    close(con.urg_sock);
                    
                    printf("connection closed\n");
                    printf("[connections count: %ld]\n", server->cons.len);
                    
                    continue;
                }

                printf("urgent data received\n");

                if (write(STDOUT_FILENO, buf, nbytes) != nbytes) {
                    fprintf(stderr, "partial/failed write\n");
                    
                    continue;
                }

                if (write(con.urg_sock, buf, nbytes) != nbytes) {
                    fprintf(stderr, "partial/failed write\n");

                    continue;
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    struct server server;

    if (argc < 2) {
        fprintf(stderr, "too few arguments.\n");
        exit(EXIT_FAILURE);
    }

    dynamic_array_init(&server.cons, CONS_CAP, sizeof(struct connection));

    if (server_listen(&server, argv[1]) == -1) {
        perror("server_listen");
        exit(EXIT_FAILURE);
    }

    server_run(&server);

    return EXIT_SUCCESS;
}