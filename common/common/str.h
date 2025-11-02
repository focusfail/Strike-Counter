#ifndef COMMON_STR_H
#define COMMON_STR_H

#include <stdlib.h>

typedef struct {
    char *c; // null terminated
    size_t l;
} str_t;

str_t str_from_cstr(const char *content);
str_t str_from_buf(const char *buffer, size_t length);

str_t str_dup(const str_t *other);

#endif // COMMON_STR_H
