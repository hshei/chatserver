#ifndef SERVER_H
#define SERVER_H

#include "datastructures.h"

typedef struct server_s {
    int kq;
    int listener_fd;
    hashmap_s *clients;
} server_s;

#endif // SERVER_H