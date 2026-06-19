#include <stdint.h>
#include <stdio.h>
#include <string.h>

    #include "client.h"

/*
    1 byte: type of data sent, see protocol.h
    4 bytes: length of the payload
    rest: payload
*/

int protocol_build_frame(uint8_t type, uint32_t payload_len, const char *payload, char *out_buf){ 
    out_buf[0] = type;
    uint32_t net_payload_len = htonl(payload_len);
    memcpy(out_buf + 1, &net_payload_len, 4);
    memcpy(out_buf + 5, payload, payload_len);
    return 5 + payload_len;
}


int protocol_parse_frame(client_s *client, uint8_t *type_out, uint32_t *payload_len_out, char *payload_out){
    uint8_t type;
    uint32_t net_payload_len;
    uint32_t payload_len;

    if (client->buf_len >= 5){
        type = client->buf[0];

        memcpy(&net_payload_len, client->buf + 1, 4);
        payload_len = ntohl(net_payload_len);

        if (client->buf_len >= payload_len + 5){
            *type_out = type;
            *payload_len_out = payload_len;
            memcpy(payload_out, client->buf + 5, payload_len);

            size_t frame_size = 5 + payload_len;
            memmove(client->buf, client->buf + frame_size, client->buf_len - frame_size);
            client->buf_len -= frame_size;
            return 1;
        }
        else return 0;
    }
    return 0;
}