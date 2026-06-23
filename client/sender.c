#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "protocol.h"

#define PORT "8888"

int main(void) {
    struct addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo("localhost", PORT, &hints, &servinfo);

    int sockfd;
    for (p = servinfo; p; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) continue;
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) { close(sockfd); continue; }
        break;
    }
    freeaddrinfo(servinfo);

    char frame[512];
    int sz;

    // login
    const char *login = "{\"username\":\"bob\"}";
    sz = protocol_build_frame(CHAT_LOGIN, strlen(login), login, frame);
    send(sockfd, frame, sz, 0);

    // join room
    const char *join = "{\"room\":\"kitchen\"}";
    sz = protocol_build_frame(CHAT_JOIN_ROOM, strlen(join), join, frame);
    send(sockfd, frame, sz, 0);

    sleep(1);

    // send three messages
    const char *msg1 = "{\"text\":\"hello everyone\"}";
    sz = protocol_build_frame(CHAT_SEND, strlen(msg1), msg1, frame);
    send(sockfd, frame, sz, 0);

    const char *msg2 = "{\"text\":\"how is it going?\"}";
    sz = protocol_build_frame(CHAT_SEND, strlen(msg2), msg2, frame);
    send(sockfd, frame, sz, 0);

    const char *msg3 = "{\"text\":\"anyone here?\"}";
    sz = protocol_build_frame(CHAT_SEND, strlen(msg3), msg3, frame);
    send(sockfd, frame, sz, 0);

    printf("bob: sent 3 messages\n");

    sleep(1);
    close(sockfd);
}
