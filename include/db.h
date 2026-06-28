#ifndef DB_H
#define DB_H

#include <sqlite3.h>
#include "cJSON.h"

int db_init(sqlite3 **db);
int db_insert_user(sqlite3 *db, const char *username, const char *password_hash);
int db_get_room_id(sqlite3 *db, const char *room);
int db_get_password(sqlite3 *db, const char *username, char *hash_out);
int db_insert_message(sqlite3 *db, const int user_id, const int room_id, const char *text);
int db_edit_message(sqlite3 *db, const char *new_text, const int msg_id, const int user_id);
int db_delete_message(sqlite3 *db, const int msg_id, const int user_id);
cJSON *db_get_history(sqlite3 *db, int room_id, int limit);
void db_close(sqlite3 *db);

#endif // DB_H