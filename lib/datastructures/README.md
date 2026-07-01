# Generic Data Structures Library in C

A type-independent data structures library using `void *` and `memcpy`. Store any data type in the same container — ints, structs, pointers. No macros, no code generation.

## Used In

- [chatserver](https://github.com/hshei/chatserver) — multi-client chat server using hashmap for client tracking and connection management

## Quick Start

```bash
make
```

Include in your project:

```c
#include "datastructures.h"
```

Compile with:

```bash
gcc your_program.c datastructures/src/*.c -I datastructures/include -o your_program
```

All functions return `ds_err_t`. Use `ds_err_str(err)` for readable error messages.

## Tests

```bash
make test
```

Runs unit tests for all data structures — covers insertion, removal, iteration, edge cases, and error handling.

```bash
make benchmark
```

Runs performance benchmarks.

All tests pass clean under AddressSanitizer (no leaks, no buffer overflows).

## API

### Vector — dynamic array, O(1) random access

| Function | Description |
|---|---|
| `vector_init(&vec, elem_size)` | Create a new vector |
| `vector_push(vec, &val)` | Append element |
| `vector_pop(vec, &out)` | Remove and return last element |
| `vector_insert(vec, &val, idx)` | Insert at index |
| `vector_remove(vec, idx)` | Remove at index |
| `vector_get(vec, idx, &out)` | Read element at index |
| `vector_set(vec, idx, &val)` | Overwrite element at index |
| `vector_foreach(vec, fn, user_data)` | Apply callback to each element |
| `vector_free(vec)` | Free all memory |

### Linked List — singly-linked, O(1) push/pop front

| Function | Description |
|---|---|
| `llist_init(&list, elem_size)` | Create a new list |
| `llist_push_front(list, &val)` | Add to front |
| `llist_push_back(list, &val)` | Add to back |
| `llist_pop_front(list, &out)` | Remove from front |
| `llist_pop_back(list, &out)` | Remove from back |
| `llist_insert(list, &val, idx)` | Insert at index |
| `llist_remove(list, idx)` | Remove at index |
| `llist_get(list, idx, &out)` | Read element at index |
| `llist_set(list, idx, &val)` | Overwrite element at index |
| `llist_foreach(list, fn, user_data)` | Apply callback to each element |
| `llist_free(list)` | Free all memory |

### HashMap — hash table, O(1) average lookup

| Function | Description |
|---|---|
| `hm_init(&map, key_size, val_size, buckets)` | Create a new map |
| `hm_insert(map, &key, &val)` | Insert or update a pair |
| `hm_get(map, &key, &out)` | Retrieve value by key |
| `hm_remove(map, &key)` | Remove a pair |
| `hm_foreach(map, fn, user_data)` | Apply callback to each pair |
| `hm_free(map)` | Free all memory |

### HashSet — hash set, O(1) average membership test

| Function | Description |
|---|---|
| `hs_init(&set, key_size, buckets)` | Create a new set |
| `hs_insert(set, &key)` | Add a key (duplicates ignored) |
| `hs_contains(set, &key, &result)` | Check if key exists |
| `hs_remove(set, &key)` | Remove a key |
| `hs_foreach(set, fn, user_data)` | Apply callback to each key |
| `hs_get_keys(set, vec)` | Collect all keys into a vector |
| `hs_free(set)` | Free all memory |

## Usage

```c
#include "datastructures.h"

int main(void) {
    // Vector
    vector_s *vec = NULL;
    vector_init(&vec, sizeof(int));
    for (int i = 0; i < 5; i++)
        vector_push(vec, &i);

    int val;
    vector_get(vec, 2, &val);  // val = 2

    // HashMap
    hashmap_s *map = NULL;
    hm_init(&map, sizeof(int), sizeof(int), 16);

    int key = 42, value = 100;
    hm_insert(map, &key, &value);

    int result;
    hm_get(map, &key, &result);  // result = 100

    // HashSet
    hashset_s *set = NULL;
    hs_init(&set, sizeof(int), 16);
    hs_insert(set, &key);

    bool exists;
    hs_contains(set, &key, &exists);  // exists = true

    vector_free(vec);
    hm_free(map);
    hs_free(set);
}
```

## Iteration

All structures support `foreach` with a callback and `void *user_data`:

```c
void sum_ints(const void *element, size_t index, void *user_data) {
    (void)index;
    *(int *)user_data += *(const int *)element;
}

int total = 0;
vector_foreach(vec, sum_ints, &total);
```

Callback signatures vary by structure:

```c
// Vector, Linked List — element + index
void (*fn)(const void *element, size_t index, void *user_data);

// HashMap — key + value
void (*fn)(const void *key, const void *value, void *user_data);

// HashSet — key only
void (*fn)(const void *key, void *user_data);
```

## Benchmarks

Mac Mini M4, 16GB RAM, `-O2`.

### Vector

```
push_back x100k          0.701 ms
sequential get x100k     0.601 ms
random get x100k         1.127 ms
middle insert x10k       9.491 ms
pop x100k (draining)     0.090 ms
```

### Linked List

``` txt
push_back x100k          5.288 ms
push_front x10k          0.477 ms
sequential get x1k       1.100 ms
middle insert x1k       57.835 ms
pop_front x100k          1.813 ms
```

### Vector vs Linked List

``` txt
Operation                   Vector     Linked List
push_back x100k            0.319ms       1.914ms
sequential get x1k         0.003ms       0.140ms
middle insert x1k          0.034ms       0.577ms
pop_front x10k             1.923ms       0.151ms
```

### HashMap

``` txt
insert x100k               5.350 ms
get x100k                  1.243 ms
remove x100k               2.785 ms
```

### HashSet

``` txt
insert x100k               3.401 ms
contains (hit) x100k       1.024 ms
contains (miss) x100k      0.615 ms
remove x100k               2.905 ms
```
