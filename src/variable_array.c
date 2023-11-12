#include "variable_array.h"

#include <malloc.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

struct variable_array *variable_array_init(void)
{
    struct variable_array *const variable_array = malloc(sizeof(struct variable_array));
    variable_array->data = NULL;
    variable_array->size = 0;
    return variable_array;
}

// void variable_array_remove_at(struct variable_array *const variable_array, size_t index)
// {
//     variable_array->data[index].data = variable_array->last_freed;
//     variable_array->last_freed = index;
// }

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

void variable_print(struct variable *variable)
{
    switch (variable->type)
    {
    case I32:
        printf("    %s: %d\n", variable->name, *(int32_t *)variable->data);
        break;
    case F32:
        printf("    %s: %f\n", variable->name, *(float *)variable->data);
        break;
    case INVALID:
        printf("Invalid variable");
        break;
    default:
        break;
    }
}

enum variable_type variable_type_from_string(const char *const string)
{
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
    case I8:
        return "i8";
    case I16:
        return "i16";
    case I32:
        return "i32";
    case I64:
        return "i64";
    case F32:
        return "f32";
    case F64:
        return "f64";
    case INVALID:
        return "Invalid";
    default:
        return "Invalid";
        break;
    }
}