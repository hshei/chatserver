CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -Ilib/datastructures/include -Ilib

SRC = src/main.c src/server.c src/protocol.c src/client.c src/handler.c lib/cJSON.c
LIB_SRC = lib/datastructures/src/vector.c \
          lib/datastructures/src/linked_list.c \
          lib/datastructures/src/hashmap.c \
          lib/datastructures/src/hashset.c

all:
	$(CC) $(CFLAGS) $(SRC) $(LIB_SRC) -o chatserver

test-client:
	$(CC) $(CFLAGS) client/client.c src/protocol.c -o test-client

clean:
	rm -f chatserver
