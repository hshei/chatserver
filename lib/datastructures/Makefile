CC = cc
CFLAGS = -Wall -Wextra -std=c11 -O2 -Iinclude -fsanitize=address
AR = ar
ARFLAGS = rcs

BUILD = build
LIB = $(BUILD)/libdatastructures.a
SRCS = src/vector.c src/error.c src/linked_list.c src/hashmap.c src/hashset.c
OBJS = $(BUILD)/vector.o $(BUILD)/error.o $(BUILD)/linked_list.o $(BUILD)/hashmap.o $(BUILD)/hashset.o
TESTS = $(BUILD)/vector_test $(BUILD)/linked_list_test $(BUILD)/hashmap_test $(BUILD)/hashset_test $(BUILD)/benchmark

all: $(BUILD) $(LIB) 

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/vector.o: src/vector.c include/datastructures.h include/vector.h
	$(CC) $(CFLAGS) -c src/vector.c -o $(BUILD)/vector.o

$(BUILD)/error.o: src/error.c include/datastructures.h include/error.h
	$(CC) $(CFLAGS) -c src/error.c -o $(BUILD)/error.o

$(BUILD)/linked_list.o: src/linked_list.c include/datastructures.h include/linked_list.h
	$(CC) $(CFLAGS) -c src/linked_list.c -o $(BUILD)/linked_list.o

$(BUILD)/hashmap.o: src/hashmap.c include/datastructures.h include/hashmap.h
	$(CC) $(CFLAGS) -c src/hashmap.c -o $(BUILD)/hashmap.o

$(BUILD)/hashset.o: src/hashset.c include/datastructures.h include/hashset.h
	$(CC) $(CFLAGS) -c src/hashset.c -o $(BUILD)/hashset.o

$(LIB): $(OBJS)
	$(AR) $(ARFLAGS) $(LIB) $(OBJS)

$(BUILD)/vector_test: tests/test_vector.c $(LIB) include/datastructures.h
	$(CC) $(CFLAGS) tests/test_vector.c -L$(BUILD) -ldatastructures -o $(BUILD)/vector_test

$(BUILD)/linked_list_test: tests/test_linked_list.c $(LIB) include/datastructures.h
	$(CC) $(CFLAGS) tests/test_linked_list.c -L$(BUILD) -ldatastructures -o $(BUILD)/linked_list_test

$(BUILD)/hashmap_test: tests/test_hashmap.c $(LIB) include/datastructures.h
	$(CC) $(CFLAGS) tests/test_hashmap.c -L$(BUILD) -ldatastructures -o $(BUILD)/hashmap_test

$(BUILD)/hashset_test: tests/test_hashset.c $(LIB) include/datastructures.h
	$(CC) $(CFLAGS) tests/test_hashset.c -L$(BUILD) -ldatastructures -o $(BUILD)/hashset_test

$(BUILD)/benchmark: tests/benchmark.c $(LIB) include/datastructures.h
	$(CC) $(CFLAGS) tests/benchmark.c -L$(BUILD) -ldatastructures -o $(BUILD)/benchmark

test: $(BUILD) $(LIB) $(TESTS)
	$(BUILD)/vector_test
	$(BUILD)/linked_list_test
	$(BUILD)/hashmap_test
	$(BUILD)/hashset_test

benchmark: $(BUILD) $(LIB) $(BUILD)/benchmark
	$(BUILD)/benchmark

.PHONY: all clean

clean:
	rm -rf $(BUILD)
