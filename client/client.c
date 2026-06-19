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
#include <unistd.h>

#include <arpa/inet.h>

#include "client.h"
#include "protocol.h"

#define PORT "8888" // the port client will be connecting to 
#define MAXDATASIZE 512 // max number of bytes we can get at once 

int main(int argc, char **argv){
    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    int rv;
    struct addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    int sockfd;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) continue;
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    uint8_t type = CHAT_LOGIN;
    const char *payload = "{\"username\":\"bob\"}";
    uint32_t payload_len = strlen(payload);
    char *frame = calloc(1, 5 + payload_len);    
    int frame_size = protocol_build_frame(type, payload_len, payload, frame);

    send(sockfd, frame, frame_size, 0);

    uint8_t type2 = CHAT_JOIN_ROOM;
    const char *payload2 = "{\"room\":\"kitchen\"}";
    uint32_t payload_len2 = strlen(payload2);
    char *frame2 = calloc(1, 5 + payload_len2);    
    int frame_size2 = protocol_build_frame(type2, payload_len2, payload2, frame2);

    send(sockfd, frame2, frame_size2, 0);
    printf("sent frame 2 (%d bytes)\n", frame_size2);
    
    sleep(1);

    free(frame);
    free(frame2);
    close(sockfd);
}