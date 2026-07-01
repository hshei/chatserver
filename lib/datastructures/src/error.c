#include "error.h"

const char *ds_err_str(ds_err_t err) {
    switch (err) {
        case DS_OK:                return "ok";
        case DS_ERR_ALLOC:         return "allocation failed";
        case DS_ERR_OUT_OF_BOUNDS: return "index out of bounds";
        case DS_ERR_EMPTY:         return "Datastructure is empty";
        case DS_ERR_INVALID_ARGUMENT: return "invalid argument";
        case DS_ERR_NOT_FOUND: return "entry not found";
        default:                   return "unknown error";
    }
}
