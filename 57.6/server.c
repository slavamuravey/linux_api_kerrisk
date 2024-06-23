#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "common.h"

#define BACKLOG 5

struct server {
    int sock;
    struct connection con;
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

    con_addr_len = sizeof(con_addr);
    server->con.sock = accept(server->sock, (struct sockaddr *)&con_addr, &con_addr_len);
    if (server->con.sock == -1) {
        close(server->sock);
        return -1;
    }

    nbytes = read(server->con.sock, &req, sizeof(req));
    if (nbytes == -1) {
        close(server->sock);
        close(server->con.sock);
        return -1;
    }

    server->con.urg_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server->con.urg_sock == -1) {
        return -1;
    }

    memset(&urg_addr, 0, sizeof(struct sockaddr_in));
    urg_addr.sin_family = con_addr.sin_family;
    urg_addr.sin_addr.s_addr = INADDR_ANY;
    urg_addr.sin_port = req.port;

    if (connect(server->con.urg_sock, (struct sockaddr *)&urg_addr, sizeof(urg_addr)) == -1) {
        close(server->sock);
        close(server->con.urg_sock);
        return -1;
    }

    memset(&res, 0, sizeof(res));
    strcpy(res.con_id, req.con_id);
    if (write(server->con.urg_sock, &res, sizeof(res)) != sizeof(res)) {
        errno = EIO;
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    struct server server;

    if (argc < 2) {
        fprintf(stderr, "too few arguments.\n");
        exit(EXIT_FAILURE);
    }

    if (server_listen(&server, argv[1]) == -1) {
        perror("server_listen");
        exit(EXIT_FAILURE);
    }

    if (server_accept(&server) == -1) {
        perror("server_accept");
        exit(EXIT_FAILURE);
    }
    
    return EXIT_SUCCESS;
}