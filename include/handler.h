void handle_login(server_s *server, int fd, const char *payload);
void handle_message(server_s *server, int fd, uint8_t type, uint32_t payload_len, const char *payload);
