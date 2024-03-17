#define _GNU_SOURCE
#include <stdlib.h>
#include <signal.h>
#include <mqueue.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "./pmsg_seqnum.h"

void handler(int s)
{
    char err[] = "unlink error\n";
    int saved_errno = errno;
    if (mq_unlink(SERVER_MQ) == -1) {
        write(STDOUT_FILENO, err, sizeof(err) - 1);
        exit(EXIT_FAILURE);
    }
    errno = saved_errno;
    fflush(NULL);
    _exit(0);
}

int main(int argc, char *argv[])
{
    mqd_t mqd_server, mqd_client;
    ssize_t num_read;
    struct mq_attr attr_server, attr_client;
    struct request req;
    struct response resp;
    char client_mq[CLIENT_MQ_NAME_LEN];
    int seq_num;
    
    attr_server.mq_msgsize = sizeof(struct request);
    attr_server.mq_maxmsg = 10;
    attr_client.mq_msgsize = sizeof(struct response);
    attr_client.mq_maxmsg = 10;
    seq_num = 0;
    
    umask(0);
    signal(SIGINT, handler);
    signal(SIGTERM, handler);

    mqd_server = mq_open(SERVER_MQ, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR, &attr_server);
    if (mqd_server == (mqd_t)-1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        num_read = mq_receive(mqd_server, (char *)&req, sizeof(struct request), NULL);
        if (num_read == -1) {
            perror("mq_receive");
            
            continue;
        }

        snprintf(client_mq, CLIENT_MQ_NAME_LEN, CLIENT_MQ_TEMPLATE, (long)req.pid);
        mqd_client = mq_open(client_mq, O_WRONLY, S_IRUSR | S_IWUSR, &attr_client);
        if (mqd_client == (mqd_t)-1) {
            perror(client_mq);
            
            continue;
        }

        resp.seq_num = seq_num;
        if (mq_send(mqd_client, (const char *)&resp, sizeof(struct response), 0) == -1) {
            fprintf(stderr, "mq_send: error writing to MQ %s\n", client_mq);
        }
            
        if (mq_close(mqd_client) == -1) {
            perror("mq_close");
        }

        seq_num += req.seq_len;
    }

    return EXIT_SUCCESS;
}
