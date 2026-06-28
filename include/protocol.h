#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include "client.h"

typedef enum {
    CHAT_LOGIN        = 0x01,
    CHAT_SEND         = 0x02,
    CHAT_EDIT         = 0x03,
    CHAT_DELETE       = 0x04,
    CHAT_JOIN_ROOM    = 0x05,
    CHAT_LEAVE_ROOM   = 0x06,
    CHAT_LIST_ROOMS   = 0x07,
    CHAT_HISTORY      = 0x08,
    CHAT_DM           = 0x09,

    CHAT_NEW_MSG      = 0x83,
    CHAT_MSG_EDITED   = 0x84,
    CHAT_MSG_DELETED  = 0x85,
    CHAT_HISTORY_RESP = 0x86,
    CHAT_LOGIN_OK     = 0x87,
    CHAT_LOGIN_FAIL   = 0x88,
    CHAT_DM_MSG       = 0x89,
} msg_type_t;


int protocol_build_frame(uint8_t type, uint32_t payload_len, const char *payload, char *out_buf);
int protocol_parse_frame(client_s *client, uint8_t *type_out, uint32_t *payload_len_out, char *payload_out);

#endif //PROTOCOL_H