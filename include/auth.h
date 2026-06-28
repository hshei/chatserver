#ifndef AUTH_H
#define AUTH_H

void auth_hash_password(const char *password, char *hash_out);
int auth_verify_password(const char *password, const char *stored_hash);

#endif // AUTH_H