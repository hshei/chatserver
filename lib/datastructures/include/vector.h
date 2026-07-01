#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include "error.h"

typedef struct vector_s {
    void *data;
    size_t size;
    size_t capacity;
    size_t elem_size;
} vector_s;

typedef void (*vector_foreach_fn)(const void *element, size_t index, void *user_data);

ds_err_t vector_init(vector_s **vector_out, size_t elem_size);
size_t vector_size(const vector_s *vector);
size_t vector_capacity(const vector_s *vector);
ds_err_t vector_push(vector_s *vector, void *element);
ds_err_t vector_pop(vector_s *vector, void *element_out);
ds_err_t vector_set(vector_s *vector, void *element, size_t index);
ds_err_t vector_insert(vector_s *vector, void *element, size_t index);
ds_err_t vector_get(const vector_s *vector, size_t index, void *element_out);
ds_err_t vector_foreach(vector_s *vec, vector_foreach_fn fn, void *user_data);
ds_err_t vector_remove(vector_s *vector, size_t index);
ds_err_t vector_free(vector_s *vector);


#endif // VECTOR_H
