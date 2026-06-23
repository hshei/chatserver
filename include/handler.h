#ifndef HANDLER_H
#define HANDLER_H

void handle_login(const char *payload, client_s *client);
void handle_message(uint8_t type, uint32_t payload_len, const char *payload, client_s *client);

#endif //HANDLER_H