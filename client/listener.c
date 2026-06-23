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

    // login
    const char *login = "{\"username\":\"alice\"}";
    char frame[256];
    int sz = protocol_build_frame(CHAT_LOGIN, strlen(login), login, frame);
    send(sockfd, frame, sz, 0);

    // join room
    const char *join = "{\"room\":\"kitchen\"}";
    sz = protocol_build_frame(CHAT_JOIN_ROOM, strlen(join), join, frame);
    send(sockfd, frame, sz, 0);

    printf("alice: joined kitchen, waiting for messages...\n");

    // listen for incoming messages
    char buf[4096];
    while (1) {
        int n = recv(sockfd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        // skip 5-byte header, print payload
        buf[n] = '\0';
        printf("received: %.*s\n", n - 5, buf + 5);
    }

    close(sockfd);
}
