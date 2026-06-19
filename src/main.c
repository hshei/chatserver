#include <sys/event.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "server.h"
#include "client.h"
#include "protocol.h"
#include "handler.h"
#include "datastructures.h"

#define PORT "8888"
#define BACKLOG 10
#define BUF_SIZE 512

static int get_listener_socket(void)
{
    int listener;     // Listening socket descriptor
    int yes=1;        // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "pollserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // If we got here, it means we didn't get bound
    if (p == NULL) {
        return -1;
    }

    freeaddrinfo(ai); // All done with this

    // Listen
    if (listen(listener, BACKLOG) == -1) {
        return -1;
    }

    return listener;
}

static void add_to_kqueue(int kq, int new_fd){
    struct kevent change;
    EV_SET(&change, new_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    if (kevent(kq, &change, 1, NULL, 0, NULL) < 0) perror("kevent: register");
}


void server_init(server_s **server_out){
    printf("Initlializing The server...\n");

    server_s *server = calloc(1, sizeof(server_s));
    hm_init(&server->clients, sizeof(int), sizeof(client_s), 64);
    server->kq = kqueue();
    server->listener_fd = get_listener_socket();
    add_to_kqueue(server->kq, server->listener_fd);
    
    *server_out = server;
}

int server_run(server_s *server){
    printf("Waiting for Connections...\n");
    struct kevent events[64];
    while (1){
        int n = kevent(server->kq, NULL, 0, events, 64, NULL);

        for (int i = 0; i < n; i++){
            int fd = events[i].ident;

            if (events[i].flags & EV_EOF) {
                // read any remaining data
                printf("DEBUG: EV_EOF for fd %d\n", fd);
                char buf[BUF_SIZE];
                int nbytes = recv(fd, buf, BUF_SIZE - 1, 0);
                printf("DEBUG: EV_EOF recv got %d bytes\n", nbytes);
                if (nbytes > 0) {
                    client_s client;
                    hm_get(server->clients, &fd, &client);
                    memcpy(client.buf + client.buf_len, buf, nbytes);
                    client.buf_len += nbytes;

                    uint8_t type;
                    uint32_t payload_len;
                    char payload[512];
                    while (protocol_parse_frame(&client, &type, &payload_len, payload) == 1) {
                        payload[payload_len] = '\0';
                        handle_message(server, fd, type, payload_len, payload);
                    }
                }
                close(fd);
                hm_remove(server->clients, &fd);
                printf("Removed Client with fd: %d successfully\n", fd);
            } else if (fd == server->listener_fd){
                // A new connection 
                printf("New client trying to connebt\n");
                struct sockaddr_storage remote_addr;
                socklen_t addr_len = sizeof(remote_addr);
        
                // new fd to be added and watched
                int new_fd = accept(server->listener_fd, (struct sockaddr *)&remote_addr, &addr_len);
                if (new_fd < 0){
                    perror("accept");
                    continue;
                }
                add_to_kqueue(server->kq, new_fd);
        
                // adding the new client to the hashmap
                client_s new_client = client_init(new_fd, remote_addr);
                hm_insert(server->clients, &new_fd, &new_client);
            } else {
                // getting the data of the client
                char buf[BUF_SIZE];
                int nbytes = recv(fd, buf, BUF_SIZE - 1, 0);
                if (nbytes == -1) { perror("recv"); continue; }
                
                printf("DEBUG: recv event for fd %d, got %d bytes\n", fd, nbytes);

                client_s client;
                hm_get(server->clients, &fd, &client);
                memcpy((client.buf + client.buf_len), buf, nbytes);
                client.buf_len += nbytes;
                // getting the frames from the client buffer
                uint8_t type;
                uint32_t payload_len;
                char payload[512];
                printf("DEBUG: buf_len before parse loop: %zu\n", client.buf_len);
                while (protocol_parse_frame(&client, &type, &payload_len, payload) == 1){
                    payload[payload_len] = '\0';
                    printf("parsing the message\n");
                    handle_message(server, fd, type, payload_len, payload);
                }
                // hm_get gives a copy of the data, so a manual update is required
                hm_insert(server->clients, &fd, &client);
            }
        }
    }
    return EXIT_SUCCESS;
}

int main(void){
    server_s *server = NULL;
    server_init(&server);
    server_run(server);

    return EXIT_SUCCESS;
}