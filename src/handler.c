#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "protocol.h"
#include "server.h"
#include "client.h"
#include "datastructures.h"
#include "cJSON.h"

void handle_login(server_s *server, int fd, const char *payload){
    cJSON *json = cJSON_Parse(payload);
    cJSON *username_json = cJSON_GetObjectItem(json, "username");
    char *username = username_json->valuestring;

    client_s client; hm_get(server->clients, &fd, &client);
    memcpy(client.username, username, strlen(username));
    client.username[strlen(username)] = '\0';

    printf("This is the username of the client of fd (%d): %s\n", fd, client.username);

    cJSON_Delete(json);
}

void handle_join_room(server_s *server, int fd, const char *payload){
    printf("Are we entering here?\n");
    cJSON *json = cJSON_Parse(payload);
    cJSON *room_json = cJSON_GetObjectItem(json, "room");
    char *room = room_json->valuestring;

    client_s client; hm_get(server->clients, &fd, &client);
    memcpy(client.room, room, strlen(room));
    client.room[strlen(room)] = '\0';

    printf("The user with fd: (%d) has entered the following room: %s\n", fd, client.room);

    cJSON_Delete(json);
}

void handle_message(server_s *server, int fd, uint8_t type, uint32_t payload_len, const char *payload){
    switch (type)
    {
        case CHAT_LOGIN:    handle_login(server, fd, payload); break;
        case CHAT_JOIN_ROOM: handle_join_room(server, fd, payload); break;
    }
}