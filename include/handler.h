#ifndef HANDLER_H
#define HANDLER_H

void handle_login(server_s *server, char *payload, client_s *client);
void handle_join_room(server_s *server, const char *payload, client_s *client);
void handle_send(server_s *server, const char *payload, client_s *client);
void handle_dm(server_s *server, const char*payload, client_s *client);
void handle_edit(server_s *server, const char *payload, client_s *client);
void handle_delete(server_s *server, const char *payload, client_s *client);
void handle_message(server_s *server, uint8_t type, uint32_t payload_len, const char *payload, client_s *client);

#endif //HANDLER_H