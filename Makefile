CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -Ilib/datastructures/include -Ilib

SRC = src/main.c src/server.c src/protocol.c src/client.c src/handler.c src/db.c src/auth.c lib/cJSON.c
LIB_SRC = lib/datastructures/src/vector.c \
          lib/datastructures/src/linked_list.c \
          lib/datastructures/src/hashmap.c \
          lib/datastructures/src/hashset.c

all:
	$(CC) $(CFLAGS) $(SRC) $(LIB_SRC) -lsqlite3 -o chatserver

client:
	$(CC) $(CFLAGS) client/client.c src/protocol.c lib/cJSON.c -o chatclient

.PHONY: all client clean

clean:
	rm -f chatserver
	rm -f chatclient
