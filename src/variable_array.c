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

void variable_array_add(struct variable_array *const variable_array, struct variable variant)
{
    variable_array->data = realloc(variable_array->data, (variable_array->size + 1) * sizeof(struct variable));
    variable_array->data[variable_array->size] = variant;
    variable_array->size++;
}

struct variable *variable_array_get_index(const struct variable_array *const variable_array, size_t index)
{
    if (index >= variable_array->size)
    {
        return NULL;
    }

    return &variable_array->data[index];
}

void variable_array_free(struct variable_array *const variable_array)
{
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
    if (strcmp(string, "i32") == 0)
    {
        return I32;
    }
    if (strcmp(string, "f32") == 0)
    {
        return F32;
    }

    return INVALID;
}

char *variable_type_to_string(enum variable_type variable_type)
{
    switch (variable_type)
    {
    case I32:
        return "i32";

    case F32:
        return "f32";
    case INVALID:
        return "Invalid";
    default:
        return "Invalid";
        break;
    }
}