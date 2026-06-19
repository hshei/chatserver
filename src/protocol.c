#include <stdio.h>
#include <string.h>

#include "client.h"

/*
    1 byte: type of data sent, see protocol.h
    4 bytes: length of the payload
    rest: payload
*/

int protocol_build_frame(client_s *client){
    return 1;
}


int protocol_parse_frame(client_s *client, uint8_t *type_out, uint32_t *payload_len_out, char *payload_out){
    uint8_t type;
    uint32_t netlen;
    uint32_t len;

    if (client->buf_len >= 5){
        type = client->buf[0];

        memcpy(&netlen, client->buf + 1, 4);
        len = ntohl(netlen);

        if (client->buf_len >= len + 5){
            *type_out = type;
            *payload_len_out = len;
            memcpy(payload_out, client->buf + 5, len);

            size_t frame_size = 5 + len;
            memmove(client->buf, client->buf + frame_size, client->buf_len - frame_size);
            client->buf_len -= frame_size;
            return 1;
        }
        else return 0;
    }
    return 0;
}