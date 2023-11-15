#ifndef AURORA_VARIABLE_ARRAY_H
#define AURORA_VARIABLE_ARRAY_H

#include <stdint.h>

enum variable_type
{
    INVALID = 0,
    BOOL,
    CHAR,
    I8,
    I16,
    I32,
    I64,
    U8,
    U16,
    U32,
    U64,
    F32,
    F64,
};

struct variable
{
    char *name;
    enum variable_type type;
    uint32_t position;
};

struct variable_array
{
    struct variable *data;
    uint32_t head;
    size_t size;
};

struct variable_array *variable_array_init(void);

int variable_array_add(struct variable_array *const variable_array, struct variable variant);

struct variable *variable_array_get_index(const struct variable_array *const variable_array, const size_t index);

int64_t variable_array_find(const struct variable_array *const variable_array, const char *const name);

void variable_array_free(struct variable_array *const variable_array);

enum variable_type variable_type_from_string(const char *const string);

char *variable_type_to_string(const enum variable_type variable_type);

uint32_t get_size_of_type(const enum variable_type variable_type);

#endif