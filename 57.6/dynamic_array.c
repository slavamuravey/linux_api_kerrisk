#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "dynamic_array.h"

void dynamic_array_init(struct dynamic_array *da, size_t cap, size_t width)
{
    da->len = 0;
    da->cap = cap;
    da->width = width;
    da->ptr = malloc(da->width * cap);
}

void dynamic_array_append(struct dynamic_array *da, void *ptr)
{
    da->len++;

    if (da->len > da->cap) {
        da->cap *= 2;
        da->ptr = realloc(da->ptr, da->width * da->cap);
    }

    memcpy((char *)da->ptr + (da->len - 1) * da->width, ptr, da->width);
}

void dynamic_array_remove(struct dynamic_array *da, size_t index)
{
    da->len--;
    memmove((char *)da->ptr + index * da->width, (char *)da->ptr + (index + 1) * da->width, (da->len - index) * da->width);
}
