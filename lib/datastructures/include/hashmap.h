#ifndef HASHMAP_H
#define HASHMAP_H

#include <stddef.h>
#include "error.h"

typedef struct hm_entry_s {
    void           *key;
    void           *value;
    struct hm_entry_s *next;
} hm_entry_s;

typedef struct {
    hm_entry_s **buckets;
    size_t       bucket_count;
    size_t       size;
    size_t       key_size;
    size_t       value_size;
} hashmap_s;

typedef void (*hm_foreach_fn)(const void *key, const void *value, void *user_data);

ds_err_t hm_init(hashmap_s **hm_out, size_t key_size, size_t value_size, size_t bucket_count);
ds_err_t hm_insert(hashmap_s *hm, void *key, void *value);
ds_err_t hm_get(const hashmap_s *hm, void *key, void *value_out);
ds_err_t hm_foreach(hashmap_s *hm, hm_foreach_fn fn, void *user_data);
ds_err_t hm_remove(hashmap_s *hm, void *key);
ds_err_t hm_free(hashmap_s *hm);

#endif // HASHMAP_H