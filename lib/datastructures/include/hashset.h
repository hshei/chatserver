#ifndef HASHSET_H
#define HASHSET_H

#include <stddef.h>
#include <stdbool.h>
#include "vector.h"
#include "error.h"

typedef struct hashset_node {
    void *key;
    struct hashset_node *next;
} hs_entry_s;

typedef struct {
    hs_entry_s **buckets;
    size_t bucket_count;
    size_t key_size;
    size_t size;
} hashset_s;

typedef void (*hs_foreach_fn) (const void *key, void *user_data);

ds_err_t hs_init(hashset_s **hs_out, size_t key_size, size_t bucket_count);
ds_err_t hs_insert(hashset_s *hs, void *key);
ds_err_t hs_contains(hashset_s *hs, void *key, bool *result);
ds_err_t hs_foreach(hashset_s *hs, hs_foreach_fn fn, void *user_data);
ds_err_t hs_get_keys(hashset_s *hs, vector_s *vector_out);
ds_err_t hs_remove(hashset_s *hs, void *key);
ds_err_t hs_free(hashset_s *hs);

#endif // HASHSET_H