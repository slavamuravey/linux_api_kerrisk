#define _GNU_SOURCE
#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "./pmsg_seqnum.h"
#include "../lib/tlpi_hdr.h"

static char client_mq[CLIENT_MQ_NAME_LEN];

static void remove_mq(void)
{
    mq_unlink(client_mq);
}

int main(int argc, char *argv[])
{
    ssize_t num_read;
    mqd_t mqd_server, mqd_client;
    struct request req;
    struct response resp;
    struct mq_attr attr_server, attr_client;
    attr_server.mq_msgsize = sizeof(struct request);
    attr_server.mq_maxmsg = 10;
    attr_client.mq_msgsize = sizeof(struct response);
    attr_client.mq_maxmsg = 10;

    umask(0);
    snprintf(client_mq, CLIENT_MQ_NAME_LEN, CLIENT_MQ_TEMPLATE, (long)getpid());
    mqd_client = mq_open(client_mq, O_RDONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, &attr_client);
    if (mqd_client == (mqd_t)-1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    if (atexit(remove_mq) != 0) {
        fprintf(stderr, "atexit error.\n");
        exit(EXIT_FAILURE);
    }

    req.pid = getpid();
    req.seq_len = (argc > 1) ? getInt(argv[1], GN_GT_0, "seq-len") : 1;

    mqd_server = mq_open(SERVER_MQ, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR, &attr_server);
    if (mqd_server == (mqd_t)-1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    if (mq_send(mqd_server, (const char *)&req, sizeof(struct request), 0) == -1) {
        fprintf(stderr, "mq_send: error writing to MQ %s\n", SERVER_MQ);
        exit(EXIT_FAILURE);
    }

    num_read = mq_receive(mqd_client, (char *)&resp, sizeof(struct response), NULL);
    if (num_read == -1) {
        perror("mq_receive");
        exit(EXIT_FAILURE);
    }
        
    printf("%d\n", resp.seq_num);

    return EXIT_SUCCESS;
}
