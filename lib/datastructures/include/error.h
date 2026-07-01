#ifndef ERROR_H
#define ERROR_H

typedef enum {
    DS_OK = 0,
    DS_ERR_ALLOC,
    DS_ERR_OUT_OF_BOUNDS,
    DS_ERR_EMPTY,
    DS_ERR_INVALID_ARGUMENT,
    DS_ERR_NOT_FOUND,
} ds_err_t;

const char *ds_err_str(ds_err_t err);

#endif // ERROR_H
