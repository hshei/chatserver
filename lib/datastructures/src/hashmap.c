#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashmap.h"
#include "error.h"

// djb2
static size_t hash(const void *key, size_t key_size, size_t bucket_count){
    const unsigned char *bytes = (const unsigned char *)key;
    size_t h = 5381;
    for (size_t i = 0; i < key_size; i++)
        h = h * 33 + bytes[i];
    return h % bucket_count;
}

static ds_err_t hm_rehash(hashmap_s *hm){
    size_t new_count = hm->bucket_count * 2;
    hm_entry_s **new_bucket;
    if ((new_bucket = calloc(new_count, sizeof(hm_entry_s *))) == NULL) return DS_ERR_ALLOC;
    for (size_t i = 0; i < hm->bucket_count; i++){
        hm_entry_s *entry = hm->buckets[i];
        if (entry == NULL) continue;
        while (entry != NULL){
            hm_entry_s *next_entry = entry->next;
            size_t new_index = hash(entry->key, hm->key_size, new_count);
            entry->next = new_bucket[new_index];
            new_bucket[new_index] = entry;
            entry = next_entry;
        }
    }
    free(hm->buckets);
    hm->buckets = new_bucket;
    hm->bucket_count = new_count;

    return DS_OK;
}

ds_err_t hm_init(hashmap_s **hm_out, size_t key_size, size_t value_size, size_t bucket_count){
    hashmap_s *hm;
    if ((hm = calloc(1, sizeof(hashmap_s))) == NULL) return DS_ERR_ALLOC;
    if ((hm->buckets = calloc(bucket_count, sizeof(hm_entry_s *))) == NULL){ 
        free(hm);
        return DS_ERR_ALLOC;
    }

    hm->bucket_count = bucket_count;
    hm->size = 0;
    hm->key_size = key_size;
    hm->value_size = value_size;

    *hm_out = hm;
    return DS_OK;
}

ds_err_t hm_insert(hashmap_s *hm, void *key, void *value){
    size_t index = hash(key, hm->key_size, hm->bucket_count);
    
    // Checking to see if the key already has a value
    for (hm_entry_s *temp = hm->buckets[index]; temp != NULL; temp = temp->next){
        if (memcmp(key, temp->key, hm->key_size) == 0){
            // it already exists, so update the value
            memcpy(temp->value, value, hm->value_size);
            return DS_OK;
        }
    }
    
    // it doesn't exist, so create a new entry
    hm_entry_s *new_entry;
    if ((new_entry = calloc(1, sizeof(hm_entry_s))) == NULL) return DS_ERR_ALLOC;
    if ((new_entry->key = calloc(1, hm->key_size)) == NULL){
        free(new_entry);
        return DS_ERR_ALLOC;
        }
    if ((new_entry->value = calloc(1, hm->value_size)) == NULL){ 
        free(new_entry->key);
        free(new_entry);
        return DS_ERR_ALLOC;
    }
    
    memcpy(new_entry->key, key, hm->key_size);
    memcpy(new_entry->value, value, hm->value_size);

    // assiging temp to the first empty slot in the array indexed
    new_entry->next = hm->buckets[index];
    hm->buckets[index] = new_entry;

    hm->size += 1;

    if ((double)hm->size / (double)hm->bucket_count > 0.75)
        return hm_rehash(hm);

    return DS_OK;
}

ds_err_t hm_get(const hashmap_s *hm, void *key, void *value_out){
    size_t index = hash(key, hm->key_size, hm->bucket_count);

    hm_entry_s *entry;
    for (entry=hm->buckets[index]; entry!=NULL; entry=entry->next){
        if (memcmp(entry->key, key, hm->key_size) == 0){
            memcpy(value_out, entry->value, hm->value_size);
            return DS_OK;
        }
    }
    return DS_ERR_NOT_FOUND;
}

ds_err_t hm_foreach(hashmap_s *hm, hm_foreach_fn fn, void *user_data){
    if ((hm == NULL) || (fn == NULL)) return DS_ERR_INVALID_ARGUMENT;

    for (size_t i = 0; i < hm->bucket_count; i++){
        hm_entry_s *entry = hm->buckets[i];
        while (entry){
            fn(entry->key, entry->value, user_data);
            entry = entry->next;
        }
    }
    return DS_OK;
}

ds_err_t hm_remove(hashmap_s *hm, void *key){
    size_t index = hash(key, hm->key_size, hm->bucket_count);

    hm_entry_s *entry;
    hm_entry_s *prev_entry = NULL;
    // If bucket is empty
    if (hm->buckets[index] == NULL) return DS_ERR_NOT_FOUND;
    // Entry at the head)
    for (entry=hm->buckets[index]; entry!=NULL; entry=entry->next){
        if (memcmp(entry->key, key, hm->key_size) == 0){
            if (prev_entry == NULL) hm->buckets[index]=entry->next;
            else prev_entry->next = entry->next;
            free(entry->key);
            free(entry->value);
            free(entry);
            hm->size -= 1;
            return DS_OK;
        }
        else prev_entry = entry;
    }
    return DS_ERR_NOT_FOUND;
}

ds_err_t hm_free(hashmap_s *hm){
    if (hm == NULL) return DS_OK;

    hm_entry_s *entry, *next_entry;
    for (size_t i =0; i < hm->bucket_count; i++){
        entry=hm->buckets[i];
        if (entry == NULL) continue;
        while (entry != NULL){
            next_entry = entry->next;
            free(entry->key);
            free(entry->value);
            free(entry);
            entry = next_entry;
        }
    }
    
    free(hm->buckets);
    free(hm);
    return DS_OK;
}