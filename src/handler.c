#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>

#include "protocol.h"
#include "server.h"
#include "client.h"
#include "db.h"
#include "auth.h"
#include "datastructures.h"
#include "cJSON.h"

typedef struct {
    const char *room;
    const char *frame;
    int frame_size;
    int sender_fd;
} broadcast_ctx;

typedef struct {
    const char *target_username;
    const char *frame;
    int frame_size;
} dm_ctx;

static void send_callback(const void *key, const void *value, void *user_data){
    broadcast_ctx *context = (broadcast_ctx *)user_data;
    client_s *client = (client_s *)value;
    
    if ((strcmp(client->room, context->room) == 0) && (client->fd != context->sender_fd)){
        send(client->fd, context->frame, context->frame_size, 0);
    }
}

static void dm_callback(const void *key, const void *value, void *user_data){
    dm_ctx *context = (dm_ctx *)user_data;
    client_s *client = (client_s *)value;
    
    if (strcmp(client->username, context->target_username) == 0){
        send(client->fd, context->frame, context->frame_size, 0);
    }
}


void handle_login(server_s *server, const char *payload, client_s *client){
    cJSON *json = cJSON_Parse(payload);
    char *username = cJSON_GetObjectItem(json, "username")->valuestring;
    char *password = cJSON_GetObjectItem(json, "password")->valuestring;

    // checking if the user already logged in
    int frame_size;
    char resp[5];
    char stored_hash[65];
    if (db_get_password(server->db, username, stored_hash) == 0){
        // user exists
        if (auth_verify_password(password, stored_hash)){
            // correct password
            strncpy(client->username, username, sizeof(client->username) - 1);
            client->username[sizeof(client->username) - 1] = '\0';  
            client->user_id = db_insert_user(server->db, username, stored_hash);
            frame_size = protocol_build_frame(CHAT_LOGIN_OK, 0, "",resp);
            send(client->fd, resp, frame_size, 0);
        } else {
            // wrong password
            frame_size = protocol_build_frame(CHAT_LOGIN_FAIL, 0, "", resp);
            send(client->fd, resp, frame_size, 0);
            cJSON_Delete(json);
            return;
        }
    } else {
        // user doesn't exist
        strncpy(client->username, username, sizeof(client->username) - 1);
        client->username[sizeof(client->username) - 1] = '\0';  
        char hashed_pass[65]; auth_hash_password(password, hashed_pass);
        client->user_id = db_insert_user(server->db, client->username, hashed_pass);
        printf("This is the username of the client of fd (%d): %s\n", client->fd, client->username);
        frame_size = protocol_build_frame(CHAT_LOGIN_OK, 0, "", resp);
        send(client->fd, resp, frame_size, 0);
    }

    cJSON_Delete(json);
}

void handle_join_room(server_s *server, const char *payload, client_s *client){
    cJSON *json = cJSON_Parse(payload);
    cJSON *room_json = cJSON_GetObjectItem(json, "room");
    char *room = room_json->valuestring;

    memcpy(client->room, room, strlen(room));
    client->room[strlen(room)] = '\0';

    client->room_id = db_get_room_id(server->db, client->room);

    printf("The user with fd: (%d) has entered the following room: %s\n", client->fd, client->room);

    cJSON_Delete(json);
}

void handle_send(server_s *server, const char *payload, client_s *client){
    cJSON *json =cJSON_Parse(payload);
    char *text = cJSON_GetObjectItem(json, "text")->valuestring;

    int msg_id = db_insert_message(server->db, client->user_id, client->room_id, text);

    // building the response
    cJSON *resp = cJSON_CreateObject();
    cJSON_AddStringToObject(resp, "from", client->username);
    cJSON_AddStringToObject(resp, "room", client->room);
    cJSON_AddStringToObject(resp, "text", text);
    cJSON_AddNumberToObject(resp, "msg_id", msg_id);

    // building the frame
    broadcast_ctx context;
    context.room = client->room;
    context.sender_fd = client->fd;
    char *resp_str = cJSON_PrintUnformatted(resp);
    uint32_t resp_len = strlen(resp_str);
    char *frame_buf = calloc(1, 5 + resp_len);
    context.frame_size = protocol_build_frame(CHAT_NEW_MSG, strlen(resp_str), resp_str, frame_buf);
    context.frame = frame_buf;

    hm_foreach(server->clients, send_callback, &context);
    free(frame_buf);
    free(resp_str);
    cJSON_Delete(json);
    cJSON_Delete(resp);
}

void handle_dm(server_s *server, const char*payload, client_s *client){
    // parsing the payload
    cJSON *json = cJSON_Parse(payload);
    char *to = cJSON_GetObjectItem(json, "to")->valuestring;
    char *text = cJSON_GetObjectItem(json, "text")->valuestring;

    // building the response
    cJSON *resp = cJSON_CreateObject();
    cJSON_AddStringToObject(resp, "from", client->username);
    cJSON_AddStringToObject(resp, "text", text);

    // building the frame 
    dm_ctx context;
    context.target_username = to;
    char *resp_str = cJSON_PrintUnformatted(resp);
    uint32_t resp_len = strlen(resp_str);
    char *frame_buf = calloc(1, 5 + resp_len);
    context.frame_size = protocol_build_frame(CHAT_DM_MSG, resp_len, resp_str, frame_buf);
    context.frame = frame_buf;

    hm_foreach(server->clients, dm_callback, &context);
    free(frame_buf);
    free(resp_str);
    cJSON_Delete(json);
    cJSON_Delete(resp);
}

void handle_edit(server_s *server, const char *payload, client_s *client){
    cJSON *json = cJSON_Parse(payload); 
    int msg_id = cJSON_GetObjectItem(json, "msg_id")->valueint;
    char *new_text = cJSON_GetObjectItem(json, "text")->valuestring;
    db_edit_message(server->db, new_text, msg_id, client->user_id);

    // building the edit response
    cJSON *resp = cJSON_CreateObject();
    cJSON_AddStringToObject(resp, "from", client->username);
    cJSON_AddStringToObject(resp, "room", client->room);
    cJSON_AddStringToObject(resp, "text", new_text);
    cJSON_AddNumberToObject(resp, "msg_id", msg_id);

    // broadcasting the frame
    broadcast_ctx context;
    context.room = client->room;
    context.sender_fd = client->fd;
    char *resp_str = cJSON_PrintUnformatted(resp);
    uint32_t resp_len = strlen(resp_str);
    char *frame_buf = calloc(1, 5 + resp_len);
    context.frame_size = protocol_build_frame(CHAT_MSG_EDITED, strlen(resp_str), resp_str, frame_buf);
    context.frame = frame_buf;

    // broadcasting the edit to the room
    hm_foreach(server->clients, send_callback, &context);
    free(frame_buf);
    free(resp_str);
    cJSON_Delete(json);
    cJSON_Delete(resp);
}

void handle_delete(server_s *server, const char *payload, client_s *client){
    cJSON *json = cJSON_Parse(payload);
    int msg_id = cJSON_GetObjectItem(json, "msg_id")->valueint;
    db_delete_message(server->db, msg_id, client->user_id);

     // building the delete response
    cJSON *resp = cJSON_CreateObject();
    cJSON_AddStringToObject(resp, "from", client->username);
    cJSON_AddStringToObject(resp, "room", client->room);
    cJSON_AddNumberToObject(resp, "msg_id", msg_id);

    // broadcasting the frame
    broadcast_ctx context;
    context.room = client->room;
    context.sender_fd = client->fd;
    char *resp_str = cJSON_PrintUnformatted(resp);
    uint32_t resp_len = strlen(resp_str);
    char *frame_buf = calloc(1, 5 + resp_len);
    context.frame_size = protocol_build_frame(CHAT_MSG_DELETED, resp_len, resp_str, frame_buf);
    context.frame = frame_buf;

    // broadcasting the delete to the room
    hm_foreach(server->clients, send_callback, &context);
    free(frame_buf);
    free(resp_str);
    cJSON_Delete(json);
    cJSON_Delete(resp);
}

void handle_history(server_s *server, client_s *client){
    cJSON *msg_json_arr = db_get_history(server->db, client->room_id, 50);

    char *msg_str = cJSON_PrintUnformatted(msg_json_arr);
    uint32_t msg_len = strlen(msg_str);
    char *msg_buf = calloc(1, 5 + msg_len);
    int frame_size = protocol_build_frame(CHAT_HISTORY_RESP, msg_len, msg_str, msg_buf);

    send(client->fd, msg_buf, frame_size, 0);
    free(msg_buf);
    free(msg_str);
    cJSON_Delete(msg_json_arr);
}

void handle_message(server_s *server, uint8_t type, uint32_t payload_len, const char *payload, client_s *client){
    switch (type)
    {
        case CHAT_LOGIN:    handle_login(server, payload, client); break;
        case CHAT_JOIN_ROOM: handle_join_room(server, payload, client); break;
        case CHAT_SEND: handle_send(server, payload, client); break;
        case CHAT_DM: handle_dm(server, payload, client); break;
        case CHAT_EDIT: handle_edit(server, payload, client); break;
        case CHAT_DELETE: handle_delete(server, payload, client); break;
        case CHAT_HISTORY: handle_history(server, client); break;
    }
}