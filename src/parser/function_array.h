#ifndef AURORA_FUNCTION_ARRAY_H
#define AURORA_FUNCTION_ARRAY_H

#include <stdint.h>

enum variable_type;

struct function
{
    char *name;
    enum variable_type *argument_types;
    size_t argument_count;
    enum variable_type *return_type;
    size_t return_count;
    int32_t call_position;
};

struct function_array
{
    struct function *data;
    size_t size;
};

struct function_array *function_array_init(void);

int64_t function_array_find(const struct function_array *const function_array, const char *const name);

void function_array_add(struct function_array *function_array, struct function function);

void function_array_free(struct function_array *function_array);

#endif