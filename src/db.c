#include <stdio.h>
#include <sqlite3.h>
#include <time.h>
#include <string.h>

#include "cJSON.h"
#include "db.h"


static const char *schema =
    "CREATE TABLE IF NOT EXISTS users ("
    "   id          INTEGER PRIMARY KEY AUTOINCREMENT,"
    "   username    TEXT UNIQUE NOT NULL,"
    "   password    TEXT NOT NULL,"
    "   created_at  INTEGER NOT NULL"
    ");"
    "CREATE TABLE IF NOT EXISTS rooms ("
    "   id          INTEGER PRIMARY KEY AUTOINCREMENT,"
    "   name        TEXT UNIQUE NOT NULL,"
    "   created_at  INTEhits si thGER NOT NULL"
    ");"
    "CREATE TABLE IF NOT EXISTS messages ("
    "   id          INTEGER PRIMARY KEY AUTOINCREMENT,"
    "   user_id     INTEGER NOT NULL REFERENCES users(id),"
    "   room_id     INTEGER NOT NULL REFERENCES rooms(id),"
    "   text        TEXT NOT NULL,"
    "   is_deleted  INTEGER DEFAULT 0,"
    "   created_at  INTEGER NOT NULL,"
    "   edited_at   INTEGER"
    ");"
    "CREATE TABLE IF NOT EXISTS direct_messages ("
    "   id          INTEGER PRIMARY KEY AUTOINCREMENT,"
    "   from_id     INTEGER NOT NULL REFERENCES users(id),"
    "   to_id       INTEGER NOT NULL REFERENCES users(id),"
    "   text        TEXT NOT NULL,"
    "   created_at  INTEGER NOT NULL"
    ");"
    "CREATE INDEX IF NOT EXISTS idx_messages_room ON messages(room_id, created_at);"
    "CREATE INDEX IF NOT EXISTS idx_dm_users ON direct_messages(from_id, to_id, created_at);";


int db_init(sqlite3 **db){
    if (sqlite3_open("chat.db", db) != SQLITE_OK){
        fprintf(stderr, "db_failed to open %s\n", sqlite3_errmsg(*db));
        return -1;
    }

    char *errmsg = NULL;
    if (sqlite3_exec(*db, schema, NULL, NULL, &errmsg) != SQLITE_OK){
        fprintf(stderr, "db: schema error: %s\n", errmsg);
        sqlite3_free(errmsg);
        return -1;
    }

    printf("db: initialized\n");
    return 0;
}

// inserts the user and returns his id, or gets the id for an existing one
int db_insert_user(sqlite3 *db, const char *username, const char *password_hash){
    sqlite3_stmt *stmt;

    // insert if not exists
    sqlite3_prepare_v2(db,
        "INSERT OR IGNORE INTO users (username, password, created_at) VALUES (?, ?, ?)",
        -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, username, -1, NULL);
    sqlite3_bind_text(stmt, 2, password_hash, -1, NULL);
    sqlite3_bind_int64(stmt, 3, time(NULL));
    sqlite3_step(stmt); // execute
    sqlite3_finalize(stmt); 

    sqlite3_prepare_v2(db,
    "SELECT id FROM users WHERE username = ?",
    -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, username, -1, NULL);

    int user_id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user_id = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    return user_id;
}

int db_get_room_id(sqlite3 *db, const char *room){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, 
    "INSERT OR IGNORE INTO rooms (name, created_at) VALUES (?, ?)",
    -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, room, -1, NULL);
    sqlite3_bind_int64(stmt, 2, time(NULL));
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
        
    sqlite3_prepare_v2(db, 
    "SELECT id FROM rooms WHERE name = ?",
    -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, room, -1, NULL);

    int room_id = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW){
        room_id = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    return room_id;
}

int db_insert_message(sqlite3 *db, const int user_id, const int room_id, const char *text){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, 
        "INSERT INTO messages (user_id, room_id, text, created_at) VALUES (?, ?, ?, ?)"
    , -1, &stmt, NULL);

    sqlite3_bind_int64(stmt, 1, user_id);
    sqlite3_bind_int64(stmt, 2, room_id);
    sqlite3_bind_text(stmt, 3, text, -1, NULL);
    sqlite3_bind_int64(stmt, 4, time(NULL));
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    int64_t msg_id = sqlite3_last_insert_rowid(db);
    return msg_id;
}

int db_get_password(sqlite3 *db, const char *username, char *hash_out){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, 
        "SELECT password FROM users where username = ?"
    , -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, username, -1, NULL);
    if (sqlite3_step(stmt) == SQLITE_ROW){
        const char *hashed_pass = (const char *)sqlite3_column_text(stmt, 0);
        strcpy(hash_out, hashed_pass);
        sqlite3_finalize(stmt);
        return 0;
    } 
    sqlite3_finalize(stmt);
    return 1;
}

int db_edit_message(sqlite3 *db, const char *new_text, const int msg_id, const int user_id){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db,
        "UPDATE messages SET text = ?, edited_at = ? WHERE id = ? AND user_id = ?"
    , -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, new_text, -1, NULL);
    sqlite3_bind_int64(stmt, 2, time(NULL));
    sqlite3_bind_int64(stmt, 3, msg_id);
    sqlite3_bind_int64(stmt, 4, user_id);
    sqlite3_step(stmt);

    int nrows_changed = sqlite3_changes(db);
    if (nrows_changed == 0){
        fprintf(stderr, "Nothing was changed...\n");
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

int db_delete_message(sqlite3 *db, const int msg_id, const int user_id){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db,
        "UPDATE messages SET is_deleted = 1 WHERE id = ? AND user_id = ?"
    , -1, &stmt, NULL);

    sqlite3_bind_int64(stmt, 1, msg_id);
    sqlite3_bind_int64(stmt, 2, user_id);
    sqlite3_step(stmt);
    int nrows_changed = sqlite3_changes(db);
    if (nrows_changed == 0){
        fprintf(stderr, "Nothing was updated...\n");
        sqlite3_finalize(stmt);
        return -1;
    }
    
    sqlite3_finalize(stmt);
    return 0;
}

cJSON *db_get_history(sqlite3 *db, int room_id, int limit){
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, 
        "SELECT m.id, u.username, m.text, m.created_at, m.edited_at, m.is_deleted "
        "FROM messages m "
        "JOIN users u ON m.user_id = u.id "
        "WHERE m.room_id = ? AND m.is_deleted = 0 "
        "ORDER BY m.created_at DESC "
        "LIMIT ?"
    , -1, &stmt, NULL);

    sqlite3_bind_int64(stmt, 1, room_id);
    sqlite3_bind_int64(stmt, 2, limit);

    cJSON *resp_arr = cJSON_CreateArray();

    while (sqlite3_step(stmt) == SQLITE_ROW){
        int msg_id = sqlite3_column_int(stmt, 0);    
        const char *username = (const char *)sqlite3_column_text(stmt, 1);  
        const char *text = (const char *)sqlite3_column_text(stmt, 2);  
        int created_at = sqlite3_column_int(stmt, 3);    
        int edited_at = sqlite3_column_int(stmt, 4);
        int is_deleted = sqlite3_column_int(stmt, 5);

        cJSON *resp = cJSON_CreateObject();
        cJSON_AddNumberToObject(resp, "msg_id", msg_id);
        cJSON_AddStringToObject(resp, "username", username);
        cJSON_AddStringToObject(resp, "text", text);
        cJSON_AddNumberToObject(resp, "created_at", created_at);
        cJSON_AddNumberToObject(resp, "edited_at", edited_at);
        cJSON_AddNumberToObject(resp, "is_deleted", is_deleted);

        cJSON_AddItemToArray(resp_arr, resp);
    }
    
    sqlite3_finalize(stmt);
    return resp_arr;
}

void db_close(sqlite3 *db){
    sqlite3_close(db);
}