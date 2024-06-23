#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include "common.h"

struct client {
    int urg_sock;
    struct connection con;
};

int client_connect(struct client *client, const char *host, const char *port)
{
    struct sockaddr_in urg_addr, peer_addr;
    socklen_t urg_addr_len;
    struct urg_handshake_req req;
    struct urg_handshake_res res;
    uuid_t binuuid;
    ssize_t nbytes;

    client->urg_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client->urg_sock == -1) {
        return -1;
    }

    if (bind(client->urg_sock, (struct sockaddr *)&urg_addr, sizeof(struct sockaddr_in)) == -1) {
        return -1;
    }

    if (listen(client->urg_sock, 0) == -1) {
        return -1;
    }

    urg_addr_len = sizeof(urg_addr);
    if (getsockname(client->urg_sock, (struct sockaddr *)&urg_addr, &urg_addr_len) == -1) {
        return -1;
    }

    memset(&peer_addr, 0, sizeof(struct sockaddr_in));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_addr.s_addr = inet_addr(host);
    peer_addr.sin_port = htons(port_from_str(port));

    client->con.sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client->con.sock == -1) {
        return -1;
    }

    if (connect(client->con.sock, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) == -1) {
        close(client->con.sock);
        return -1;
    }

    memset(&req, 0, sizeof(req));
    uuid_generate_random(binuuid);
    uuid_unparse_lower(binuuid, req.con_id);
    req.port = urg_addr.sin_port;
    if (write(client->con.sock, &req, sizeof(req)) != sizeof(req)) {
        errno = EIO;
        return -1;
    }

    client->con.urg_sock = accept(client->urg_sock, NULL, NULL);
    if (client->con.urg_sock == -1) {
        close(client->con.sock);
        close(client->urg_sock);
        return -1;
    }

    nbytes = read(client->con.urg_sock, &res, sizeof(res));
    if (nbytes == -1) {
        close(client->con.sock);
        close(client->urg_sock);
        close(client->con.urg_sock);
        return -1;
    }

    if (strcmp(req.con_id, res.con_id)) {
        errno = EACCES;
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    struct client client;

    if (argc < 3) {
        fprintf(stderr, "too few arguments.\n");
        exit(EXIT_FAILURE);
    }

    if (client_connect(&client, argv[1], argv[2]) == -1) {
        perror("client_connect");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}