#ifndef PMSG_SEQNUM_H
#define PMSG_SEQNUM_H

#define SERVER_MQ "/mq_server"
#define CLIENT_MQ_TEMPLATE "/mq_client.%ld"
#define CLIENT_MQ_NAME_LEN (sizeof(CLIENT_MQ_TEMPLATE) + 20)

struct request {
    pid_t pid;
    int seq_len;
};

struct response {
    int seq_num;
};

#endif