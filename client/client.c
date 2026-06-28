#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/event.h>

#include "protocol.h"
#include "cJSON.h"

#define PORT "8888"
#define BUF_SIZE 4096

static void add_to_kqueue(int kq, int new_fd) {
    struct kevent change;
    EV_SET(&change, new_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    if (kevent(kq, &change, 1, NULL, 0, NULL) < 0) perror("kevent: register");
}

static void send_json(int sockfd, uint8_t type, cJSON *json) {
    char *str = cJSON_PrintUnformatted(json);
    uint32_t len = strlen(str);
    char *frame = calloc(1, 5 + len);
    int sz = protocol_build_frame(type, len, str, frame);
    send(sockfd, frame, sz, 0);
    free(frame);
    free(str);
    cJSON_Delete(json);
}


int main(void) {
    // getting sockfd
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

    // getting kq
    int kq = kqueue();
    add_to_kqueue(kq, sockfd);
    add_to_kqueue(kq, STDIN_FILENO);

    // waiting for socket and stdin 
    struct kevent events[64];
    while (1) {
        int n = kevent(kq, NULL, 0, events, 64, NULL);
        for (int i = 0; i < n; i++){
            int fd = events[i].ident;

            if (fd == STDIN_FILENO){
                char buf[BUF_SIZE];
                int nbytes = read(fd, buf, BUF_SIZE - 1);
                if (nbytes == -1) {perror("read"); continue;}
                // removing the trailing \n
                buf[nbytes - 1] = '\0'; 
                char username[32], password[32], room[32];
                int msg_id;
                if (sscanf(buf, "/login %31s %31s", username, password) == 2){
                    cJSON *resp = cJSON_CreateObject();
                    cJSON_AddStringToObject(resp, "username", username);
                    cJSON_AddStringToObject(resp, "password", password);
                    send_json(sockfd, CHAT_LOGIN, resp);

                } else if (sscanf(buf, "/join %31s", room) == 1){
                    cJSON *resp = cJSON_CreateObject();
                    cJSON_AddStringToObject(resp, "room", room);
                    send_json(sockfd, CHAT_JOIN_ROOM, resp);

                } else if (sscanf(buf, "/dm %31s", username) == 1) {
                    char *message = buf + 4 + strlen(username) + 1;
                    if (strlen(message) > 0) {
                        cJSON *resp = cJSON_CreateObject();
                        cJSON_AddStringToObject(resp, "to", username);
                        cJSON_AddStringToObject(resp, "text", message);
                        send_json(sockfd, CHAT_DM, resp);
                    }
                } else if (sscanf(buf, "/edit %d", &msg_id) == 1){
                    char *new_message = strchr(buf + 6, ' ');
                    if (new_message) new_message++; else continue;
                    if (strlen(new_message) > 0){
                        cJSON *resp = cJSON_CreateObject();
                        cJSON_AddNumberToObject(resp, "msg_id", msg_id);
                        cJSON_AddStringToObject(resp, "text", new_message);
                        send_json(sockfd, CHAT_EDIT, resp);
                    }
                } else if (sscanf(buf, "/delete %d", &msg_id) == 1) {
                    cJSON *resp = cJSON_CreateObject();
                    cJSON_AddNumberToObject(resp, "msg_id", msg_id);
                    send_json(sockfd, CHAT_DELETE, resp);
                } else if (strcmp(buf, "/history") == 0) {
                    char frame[5];
                    int sz = protocol_build_frame(CHAT_HISTORY, 0, "", frame);
                    send(sockfd, frame, sz, 0);
                } else {
                    if (strlen(buf) > 0){
                        cJSON *resp = cJSON_CreateObject();
                        cJSON_AddStringToObject(resp, "text", buf);
                        send_json(sockfd, CHAT_SEND, resp);
                    }
                }
            } else if (fd == sockfd){
                char recv_buf[BUF_SIZE];
                int nbytes = recv(sockfd, recv_buf, sizeof(recv_buf), 0);
                if (nbytes <= 0){
                    printf("disconnected from server\n");
                    close(sockfd);
                    return 0;
                }
                // skip the 5-byte header, print the payload
                recv_buf[nbytes] = '\0';
                printf("%.*s\n", nbytes, recv_buf + 5);
            }
        }
    }
}
