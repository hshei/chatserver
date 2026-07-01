#include "vector.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initalizing new vector
ds_err_t vector_init(vector_s **vector_out, size_t elem_size){
    vector_s *vector;
    if ((vector = calloc(1, sizeof(vector_s))) == NULL)  return DS_ERR_ALLOC;

    vector->elem_size = elem_size;
    vector->size = 0;
    vector->capacity = 16;

    if ((vector->data = calloc(vector->capacity, vector->elem_size)) == NULL){
        free(vector); 
        return DS_ERR_ALLOC;
    }

    *vector_out = vector;
    return DS_OK;
}

size_t vector_size(const vector_s *vector){
    return vector->size;
}

size_t vector_capacity(const vector_s *vector){
    return vector->capacity;
}

// Pushing to the end of the vector
ds_err_t vector_push(vector_s *vector, void *element){
    // Reallocating when the capacity is not enough
    if (vector->size >= vector->capacity){
        size_t new_capacity = vector->capacity + (vector->capacity / 2); 
        void *tmp = realloc(vector->data, new_capacity * vector->elem_size);

        if (tmp == NULL) return DS_ERR_ALLOC;

        vector->data = tmp;
        vector->capacity = new_capacity;
    }
 
    // Casting to (char *) so I can do pointer arithmetic, since you can't do that with (void *), because the compiler does not know the type of data it points to
    // You will see this very often, since it is the C way of working with single-bytes.
    char *base = (char *)vector->data;
    char *dest = base + vector->size * vector->elem_size;
    memcpy(dest, element, vector->elem_size);

    vector->size += 1;

    return DS_OK;
}

// Popping from the end of the vector
ds_err_t vector_pop(vector_s *vector, void *element_out){
    if (vector->size == 0) return DS_ERR_EMPTY;
    
    vector->size -= 1;

    char *base = (char *)vector->data;
    char *element = base + vector->size * vector->elem_size;
    
    if (element_out != NULL) memcpy(element_out, element, vector->elem_size);    
    return DS_OK;
}

// Setting (updating) the element at certain index
ds_err_t vector_set(vector_s *vector, void *element, size_t index){
    if (index >= vector->size) return DS_ERR_OUT_OF_BOUNDS;
       

    char *base = (char *)vector->data;
    char *old_element = base + index * vector->elem_size;
    memcpy(old_element, element, vector->elem_size);

    return DS_OK;
}

// Inserting a new element at a certain index
ds_err_t vector_insert(vector_s *vector, void *element, size_t index){
    if (index > vector->size) return DS_ERR_OUT_OF_BOUNDS;

    if (vector->size >= vector->capacity){
        size_t new_capacity = vector->capacity + (vector->capacity / 2);
        void *tmp = realloc(vector->data, new_capacity * vector->elem_size);

        if (tmp == NULL) return DS_ERR_ALLOC;
        
        vector->data = tmp;
        vector->capacity = new_capacity;
    }

    char *base = (char *)vector->data;
    char *index_element = base + index * vector->elem_size;

    // Moving all the items to the right.    
    size_t elements_to_copy = vector->size - index;

    memmove(
        index_element + vector->elem_size,
        index_element,
        elements_to_copy * vector->elem_size
    );

    // Copying the new element onto the index.
    memcpy(index_element, element, vector->elem_size);

    vector->size += 1;

    return DS_OK;
}

// Getting the value of certain index, without modifying the vector
ds_err_t vector_get(const vector_s *vector, size_t index, void *element_out){
    if (index >= vector->size) return DS_ERR_OUT_OF_BOUNDS;

    char *base = (char *)vector->data;
    char *element = base + index * vector->elem_size;

    memcpy(element_out, element, vector->elem_size);
    return DS_OK;
}

ds_err_t vector_foreach(vector_s *vector, vector_foreach_fn fn, void *user_data){
    if ((vector == NULL) || (fn == NULL)) return DS_ERR_INVALID_ARGUMENT;

    for (size_t i = 0; i < vector->size; i++){
        char *base = (char *)vector->data;
        char *element = base + i * vector->elem_size;
        fn(element, i, user_data);
    }
    return DS_OK;
}

// Removing an element at a certain index
ds_err_t vector_remove(vector_s *vector, size_t index){
    if (index >= vector->size) return DS_ERR_OUT_OF_BOUNDS;

    char *base = (char *)vector->data;
    char *index_element = base + index * vector->elem_size;
    size_t elements_to_copy = vector->size - index - 1;
    if (elements_to_copy > 0) {
        memmove(index_element,
                index_element + vector->elem_size,
                elements_to_copy * vector->elem_size);
    }
    vector->size -= 1;

    return DS_OK;
}

// Freeing the allocated memory for the given vector
ds_err_t vector_free(vector_s *vector){
    if (vector == NULL){
        return DS_OK;
    }

    if (vector->data != NULL){
        free(vector->data);
        vector->data = NULL;
    }

    free(vector);
    return DS_OK;
}
