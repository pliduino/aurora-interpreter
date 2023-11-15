#include "parser/function_array.h"

#include <malloc.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

struct function_array *function_array_init(void)
{
    struct function_array *const function_array = malloc(sizeof(function_array));
    function_array->size = 0;
    function_array->data = NULL;

    return function_array;
}

int64_t function_array_find(const struct function_array *const function_array, const char *const name)
{
    for (uint32_t i = 0; i < function_array->size; i++)
    {
        // TODO: Sometimes crashes
        if (strcmp(function_array->data[i].name, name) == 0)
        {
            return i;
        }
    }

    return -1;
}

void function_array_add(struct function_array *function_array, struct function function)
{
    if (function_array_find(function_array, function.name) >= 0)
    {
        return -1;
    }

    function_array->data = realloc(function_array->data, (function_array->size + 1) * sizeof(struct function));
    function_array->data[function_array->size] = function;
    function_array->size++;
    return 0;
}

void function_array_free(struct function_array *function_array)
{
    if (function_array == NULL)
    {
        return;
    }

    for (size_t i = 0; i < function_array->size; i++)
    {
        free(function_array->data[i].name);
    }

    free(function_array->data);

    // TODO: Sometimes crashes
    // free(function_array);
}