#ifndef DB_H
#define DB_H

#include <sqlite3.h>

int db_init(sqlite3 **db);
int db_insert_user(sqlite3 *db, const char *username);
int db_get_room_id(sqlite3 *db, const char *room);
int db_insert_message(sqlite3 *db, const int user_id, const int room_id, const char *text);
void db_close(sqlite3 *db);

#endif // DB_H