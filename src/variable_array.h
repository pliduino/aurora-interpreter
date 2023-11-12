#ifndef AURORA_VARIABLE_ARRAY_H
#define AURORA_VARIABLE_ARRAY_H

#include <stdint.h>

enum variable_type
{
    INVALID = 0,
    I8,
    I16,
    I32,
    I64,
    F32,
    F64,
};

struct variable
{
    char *name;
    enum variable_type type;
    void *data;
};

struct variable_array
{
    struct variable *data;
    void *last_freed;
    size_t size;
};

struct variable_array *variable_array_init(void);

void variable_array_remove_at(struct variable_array *const variable_array, size_t index);

int variable_array_add(struct variable_array *const variable_array, struct variable variant);

struct variable *variable_array_get_index(const struct variable_array *const variable_array, size_t index);

int64_t variable_array_find(const struct variable_array *const variable_array, const char *const name);

void variable_array_free(struct variable_array *const variable_array);

void variable_print(struct variable *variable);

enum variable_type variable_type_from_string(const char *const string);

char *variable_type_to_string(enum variable_type variable_type);

#endif