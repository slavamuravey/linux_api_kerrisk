#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <unistd.h>

struct dynamic_array {
    size_t len;
    size_t cap;
    size_t width;
    void *ptr;
};

void dynamic_array_init(struct dynamic_array *da, size_t cap, size_t width);
void dynamic_array_append(struct dynamic_array *da, void *ptr);
void dynamic_array_remove(struct dynamic_array *da, size_t index);

#endif
