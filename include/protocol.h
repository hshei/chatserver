typedef enum {
    MSG_LOGIN        = 0x01,
    MSG_SEND         = 0x02,
    MSG_EDIT         = 0x03,
    MSG_DELETE       = 0x04,
    MSG_JOIN_ROOM    = 0x05,
    MSG_LEAVE_ROOM   = 0x06,
    MSG_LIST_ROOMS   = 0x07,
    MSG_HISTORY      = 0x08,
    MSG_DM           = 0x09,
} msg_type_t;