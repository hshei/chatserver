#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>

#include <arpa/inet.h>

#include "client.h"
#include "protocol.h"

#define PORT "8888" // the port client will be connecting to 
#define MAXDATASIZE 512 // max number of bytes we can get at once 

client_s client_init(int new_fd, struct sockaddr_storage remote_addr){
    client_s new_client;
    new_client.fd = new_fd;
    if (remote_addr.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&remote_addr;
        inet_ntop(AF_INET, &s->sin_addr, new_client.addr, sizeof(new_client.addr));
    }
    new_client.buf_len = 0;
    return new_client;
}
