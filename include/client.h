#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>
#include "datastructures.h"

typedef struct {
    int fd;
    char addr[INET6_ADDRSTRLEN];
    char buf[4096];
    char username[32];
    char room[32];
    size_t buf_len;
} client_s;


client_s client_init(int new_fd, struct sockaddr_storage remote_addr);

#endif // CLIENT_H