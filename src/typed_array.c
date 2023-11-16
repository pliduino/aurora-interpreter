#include "typed_array.h"

#include <malloc.h>
#include <stdio.h>

struct typed_array *typed_array_init(enum typed_array_type type, int size)
{
    struct typed_array *array;
    array = malloc(sizeof(struct typed_array));

    switch (type)
    {
    case INT:
        break;
    case FLOAT:
        /* code */
        break;
    default:
        fprintf(stderr, "%s:%d - Type provided for array is invalid!", __FILE__, __LINE__);
        break;
    }
    array->size = size;
    return array;
}