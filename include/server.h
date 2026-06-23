#ifndef SERVER_H
#define SERVER_H

#include "datastructures.h"

typedef struct server_s {
    int kq;
    int listener_fd;
    hashmap_s *clients;
} server_s;

int get_listener_socket(void);
void add_to_kqueue(int kq, int new_fd);
void server_init(server_s **server_out);
int server_run(server_s *server);

#endif // SERVER_H