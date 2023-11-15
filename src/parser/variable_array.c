#include "variable_array.h"

#include <malloc.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

struct variable_array *variable_array_init(void)
{
    struct variable_array *const variable_array = malloc(sizeof(struct variable_array));
    variable_array->data = NULL;
    variable_array->head = 0;
    variable_array->size = 0;
    return variable_array;
}

/// @brief a \n a
/// @param variable_array
/// @param variable
/// @return -1 if variable already exists 0 if successful
int variable_array_add(struct variable_array *const variable_array, struct variable variable)
{
    if (variable_array_find(variable_array, variable.name) >= 0)
    {
        return -1;
    }

    variable_array->data = realloc(variable_array->data, (variable_array->size + 1) * sizeof(struct variable));
    variable_array->data[variable_array->size] = variable;
    variable_array->size++;
    return 0;
}

uint32_t get_size_of_type(enum variable_type variable_type)
{
    switch (variable_type)
    {
    case BOOL:
        return 1;
    case CHAR:
        return 1;
    case I8:
        return 1;
    case I16:
        return 2;
    case I32:
        return 4;
    case I64:
        return 8;
    case U8:
        return 1;
    case U16:
        return 2;
    case U32:
        return 4;
    case U64:
        return 8;
    case F32:
        return 4;
    case F64:
        return 8;
    case INVALID:
        return 0;
    default:
        return 0;
        break;
    }
}

struct variable *variable_array_get_index(const struct variable_array *const variable_array, size_t index)
{
    if (index >= variable_array->size)
    {
        return NULL;
    }

    return &variable_array->data[index];
}

int64_t variable_array_find(const struct variable_array *const variable_array, const char *const name)
{
    for (uint32_t i = 0; i < variable_array->size; i++)
    {
        if (strcmp(variable_array->data[i].name, name) == 0)
        {
            return i;
        }
    }

    return -1;
}

void variable_array_free(struct variable_array *const variable_array)
{
    if (variable_array == NULL)
    {
        return;
    }

    for (size_t i = 0; i < variable_array->size; i++)
    {
        free(variable_array->data[i].name);
        free(variable_array->data[i].data);
    }

    free(variable_array->data);
    free(variable_array);
}

enum variable_type variable_type_from_string(const char *const string)
{
    if (strcmp(string, "bool") == 0)
    {
        return BOOL;
    }
    if (strcmp(string, "char") == 0)
    {
        return CHAR;
    }
    if (strcmp(string, "i8") == 0)
    {
        return I8;
    }
    if (strcmp(string, "i16") == 0)
    {
        return I16;
    }
    if (strcmp(string, "i32") == 0)
    {
        return I32;
    }
    if (strcmp(string, "i64") == 0)
    {
        return I64;
    }
    if (strcmp(string, "u8") == 0)
    {
        return U8;
    }
    if (strcmp(string, "u16") == 0)
    {
        return U16;
    }
    if (strcmp(string, "u32") == 0)
    {
        return U32;
    }
    if (strcmp(string, "u64") == 0)
    {
        return U64;
    }
    if (strcmp(string, "f32") == 0)
    {
        return F32;
    }
    if (strcmp(string, "f64") == 0)
    {
        return F64;
    }

    return INVALID;
}

char *variable_type_to_string(enum variable_type variable_type)
{
    switch (variable_type)
    {
    case BOOL:
        return "bool";
    case CHAR:
        return "char";
    case I8:
        return "i8";
    case I16:
        return "i16";
    case I32:
        return "i32";
    case I64:
        return "i64";
    case U8:
        return "u8";
    case U16:
        return "u16";
    case U32:
        return "u32";
    case U64:
        return "u64";
    case F32:
        return "f32";
    case F64:
        return "f64";
    case INVALID:
        return "Invalid";
    default:
        return "Not Implemented";
        break;
    }
}