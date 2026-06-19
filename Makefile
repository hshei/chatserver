CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -Ilib/datastructures/include

SRC = src/main.c src/server.c
LIB_SRC = lib/datastructures/src/vector.c \
          lib/datastructures/src/linked_list.c \
          lib/datastructures/src/hashmap.c \
          lib/datastructures/src/hashset.c

all:
	$(CC) $(CFLAGS) $(SRC) $(LIB_SRC) -o chatserver

clean:
	rm -f chatserver
