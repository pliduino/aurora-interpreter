#ifndef AURORA_VARIABLE_ARRAY_H
#define AURORA_VARIABLE_ARRAY_H

enum variable_type
{
    INVALID = 0,
    I32,
    F32,
};

struct variable
{
    char *name;
    void *data;
    enum variable_type type;
};

struct variable_array
{
    struct variable *data;
    size_t size;
};

struct variable_array *variable_array_init(void);

void variable_array_add(struct variable_array *const variable_array, struct variable variant);

struct variable *variable_array_get_index(const struct variable_array *const variable_array, size_t index);

int variable_array_find(const struct variable_array *const variable_array, const char *const name);

void variable_array_free(struct variable_array *const variable_array);

void variable_print(struct variable *variable);

enum variable_type variable_type_from_string(const char *const string);

char *variable_type_to_string(enum variable_type variable_type);

#endif