#ifndef AURORA_FUNCTION_ARRAY_H
#define AURORA_FUNCTION_ARRAY_H

#include <stdint.h>

enum variable_type;

struct argument
{
    char *name;
    enum variable_type variable_type;
};

struct function
{
    char *name;
    struct argument *arguments;
    size_t argument_count;
    enum variable_type *return_type;
    size_t return_count;
    uint32_t call_position;
    uint8_t iteration_depth;
};

struct function_array
{
    struct function *data;
    size_t size;
};

struct function_array *function_array_init(void);

int64_t function_array_find(const struct function_array *const function_array, const char *const name);

int function_array_add(struct function_array *const restrict function_array, const struct function function);

void function_array_free(struct function_array *const restrict function_array);

#endif