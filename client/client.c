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

    printf("Connected to the server...\n");
    // waiting for socket and stdin 
    struct kevent events[64];
    while (1) {
        int n = kevent(kq, NULL, 0, events, 64, NULL);
        for (int i = 0; i < n; i++){
            int fd = events[i].ident;

            if (fd == STDIN_FILENO){
                char buf[BUF_SIZE];
                memset(buf, 0, BUF_SIZE);
                int nbytes = read(fd, buf, BUF_SIZE - 1);
                if (nbytes == -1) {perror("read"); continue;}
                // removing the trailing \n
                buf[nbytes - 1] = '\0'; 
                char login_user[32], login_pass[32];
                char room[32];
                char dm_target[32];
                int msg_id;
                if (sscanf(buf, "/login %31s %31s", login_user, login_pass) == 2){
                    printf("logging in as %s...\n", login_user);
                    cJSON *resp = cJSON_CreateObject();
                    cJSON_AddStringToObject(resp, "username", login_user);
                    cJSON_AddStringToObject(resp, "password", login_pass);
                    send_json(sockfd, CHAT_LOGIN, resp);

                } else if (sscanf(buf, "/join %31s", room) == 1){
                    cJSON *resp = cJSON_CreateObject();
                    cJSON_AddStringToObject(resp, "room", room);
                    send_json(sockfd, CHAT_JOIN_ROOM, resp);
                    printf("joined #%s\n", room);

                }  else if (sscanf(buf, "/delete %d", &msg_id) == 1) {
                    cJSON *resp = cJSON_CreateObject();
                    cJSON_AddNumberToObject(resp, "msg_id", msg_id);
                    send_json(sockfd, CHAT_DELETE, resp);
                    printf("deleting message #%d...\n", msg_id); 

                } else if (sscanf(buf, "/edit %d", &msg_id) == 1){
                    char *new_message = strchr(buf + 6, ' ');
                    if (new_message) new_message++; else continue;
                    if (strlen(new_message) > 0){
                        cJSON *resp = cJSON_CreateObject();
                        cJSON_AddNumberToObject(resp, "msg_id", msg_id);
                        cJSON_AddStringToObject(resp, "text", new_message);
                        send_json(sockfd, CHAT_EDIT, resp);
                        printf("editing message #%d...\n", msg_id);
                    }
                    
                } else if (sscanf(buf, "/dm %31s", dm_target) == 1) {
                        int offset = 4 + strlen(dm_target);
                        if (buf[offset] == ' ' && strlen(buf + offset + 1) > 0) {
                            char *message = buf + offset + 1;
                            cJSON *resp = cJSON_CreateObject();
                            cJSON_AddStringToObject(resp, "to", dm_target);
                            cJSON_AddStringToObject(resp, "text", message);
                            send_json(sockfd, CHAT_DM, resp);
                            printf("[DM to %s] %s\n", dm_target, message);
                        } else {
                            printf("usage: /dm <user> <message>\n");
                        }
                } else if (strcmp(buf, "/history") == 0) {
                    char frame[5];
                    int sz = protocol_build_frame(CHAT_HISTORY, 0, "", frame);
                    send(sockfd, frame, sz, 0);

                } else if (strcmp(buf, "/help") == 0) {
                    printf("Commands: /login /join /dm /edit /delete /history\n");

                }else {
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
                uint8_t type = recv_buf[0];
                recv_buf[nbytes] = '\0';
                char *payload = recv_buf + 5;

                switch (type) {
                    case CHAT_LOGIN_OK:
                        printf("logged in successfully\n");
                        break;
                    case CHAT_LOGIN_FAIL:
                        printf("login failed: wrong password\n");
                        break;
                    default: {
                        cJSON *json = cJSON_Parse(payload);
                        if (!json) { printf("%s\n", payload); break; }

                        switch (type) {
                            case CHAT_NEW_MSG: {
                                const char *from = cJSON_GetObjectItem(json, "from")->valuestring;
                                const char *text = cJSON_GetObjectItem(json, "text")->valuestring;
                                int msg_id = cJSON_GetObjectItem(json, "msg_id")->valueint;
                                printf("[%s] (#%d) %s\n", from, msg_id, text);
                                break;
                            }
                            case CHAT_DM_MSG: {
                                const char *from = cJSON_GetObjectItem(json, "from")->valuestring;
                                const char *text = cJSON_GetObjectItem(json, "text")->valuestring;
                                printf("[DM from %s] %s\n", from, text);
                                break;
                            }
                            case CHAT_MSG_EDITED: {
                                int msg_id = cJSON_GetObjectItem(json, "msg_id")->valueint;
                                const char *text = cJSON_GetObjectItem(json, "text")->valuestring;
                                printf("(message %d edited) %s\n", msg_id, text);
                                break;
                            }
                            case CHAT_MSG_DELETED: {
                                int msg_id = cJSON_GetObjectItem(json, "msg_id")->valueint;
                                printf("(message %d deleted)\n", msg_id);
                                break;
                            }
                            case CHAT_HISTORY_RESP: {
                                printf("--- History ---\n");
                                cJSON *arr = cJSON_Parse(payload);
                                int size = cJSON_GetArraySize(arr);
                                for (int j = size - 1; j >= 0; j--) {
                                    cJSON *msg = cJSON_GetArrayItem(arr, j);
                                    const char *from = cJSON_GetObjectItem(msg, "username")->valuestring;
                                    const char *text = cJSON_GetObjectItem(msg, "text")->valuestring;
                                    int msg_id = cJSON_GetObjectItem(msg, "msg_id")->valueint;
                                    printf("[%s] (#%d) %s\n", from, msg_id, text);
                                }
                                printf("--- End ---\n");
                                cJSON_Delete(arr);
                                break;
                            }
                        }
                    cJSON_Delete(json);
                    }
                }
            }
        }
    }
}

