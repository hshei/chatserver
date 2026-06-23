CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -Ilib/datastructures/include -Ilib

SRC = src/main.c src/server.c src/protocol.c src/client.c src/handler.c src/db.c lib/cJSON.c
LIB_SRC = lib/datastructures/src/vector.c \
          lib/datastructures/src/linked_list.c \
          lib/datastructures/src/hashmap.c \
          lib/datastructures/src/hashset.c

all:
	$(CC) $(CFLAGS) $(SRC) $(LIB_SRC) -lsqlite3 -o chatserver

test-client:
	$(CC) $(CFLAGS) client/client.c src/protocol.c -o test-client

listener:
	$(CC) $(CFLAGS) client/listener.c src/protocol.c -o listener

sender:
	$(CC) $(CFLAGS) client/sender.c src/protocol.c -o sender

clean:
	rm -f chatserver
	rm -f test-client
	rm -f listener
	rm -f sender
	rm -f chat.db
