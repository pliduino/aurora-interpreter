#include "program.h"

#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>

#include "variable_array.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

void program_print_error(const struct program *const program, const char *const error_format, ...)
{
    va_list argptr;
    va_start(argptr, error_format);
    fprintf(stderr, "   %s::%lld: ", program->file_path, program->cur_line);
    vfprintf(stderr, error_format, argptr);
    va_end(argptr);
}

struct program *program_init(const char *const file_path)
{
    struct program *program = malloc(sizeof(struct program));
    program->file_path = file_path;
    errno_t error = fopen_s(&(program->fptr), file_path, "r");

    if (error != 0)
    {
        fprintf(stderr, "%s:%d - File does not exist!\n", __FILE__, __LINE__);
        return NULL;
    }

    program->variables = variable_array_init();
    program->cur_line = 0;
    return program;
}

/// @brief Adds variable to program
/// @param program
/// @param variable_name A copy is made, please free the original one if needed
/// @param variable_type
void program_add_var(const struct program *program, const char *const variable_name, enum variable_type variable_type)
{
    struct variable variable;
    size_t string_size = sizeof(char) * (strlen(variable_name) + 1);
    variable.name = malloc(string_size);
    strcpy_s(variable.name, string_size, variable_name);

    variable.type = variable_type;
    switch (variable_type)
    {
    case I32:
        int32_t *data_int = malloc(sizeof(int32_t));
        *data_int = 0;
        variable.data = data_int;
        break;
    case F32:
        float *data_float = malloc(sizeof(float));
        *data_float = 0.0;
        variable.data = data_float;
        break;
    case INVALID:
        break;
    default:
        break;
    }

    variable_array_add(program->variables, variable);
}

struct variable *program_get_var(const struct program *const program, const char *const variable_name)
{
    for (size_t i = 0; i < program->variables->size; i++)
    {
        if (strcmp(variable_name, program->variables->data[i].name) == 0)
        {
            return &program->variables->data[i];
        }
    }

    return NULL;
}

int program_set_var(const struct program *const program, const char *const variable_name, const struct variable assign)
{
    struct variable *variable = program_get_var(program, variable_name);

    if (variable == NULL)
    {
        program_print_error(program, "%s does not exist! Creating new variable and assigning value...\n", variable_name);
        program_add_var(program, variable_name, assign.type);
        return -2;
    }

    if (variable->type == assign.type)
    {
        variable->data = assign.data;
        return 0;
    }

    program_print_error(program, "Trying to assign %s to %s:%s!\n", variable_type_to_string(assign.type), variable_type_to_string(variable->type), variable->name);
    return -1;
}

void program_run(struct program *const program)
{
    clock_t exec_time;
    exec_time = clock();

    struct token_list *token_list = lex_file(program->fptr);

    char *parsed_program = parse_tokens(token_list);
    token_list_destroy(token_list);

    if (parsed_program == NULL)
    {
        fprintf(stderr, "%s:%d - Error while parsing!\n", __FILE__, __LINE__);
        return;
    }

    for (program->cur_line = 0; strncmp(&parsed_program[program->cur_line * WORD_SIZE], EOP, COMMAND_BYTES) == 0; program->cur_line++)
    {
        if (strncmp(&parsed_program[0 * WORD_SIZE], O_CREATE_VAR, COMMAND_BYTES) == 0)
        {
            printf("Creating var");
        }
    }

    free(parsed_program);

    exec_time = clock() - exec_time;
    double time_taken = (((double)exec_time) / CLOCKS_PER_SEC) * 1000;
    printf("\nExecution time: %.0fms\n", time_taken);
}

/// @brief Closes file and frees variables
/// @param program
void program_close(const struct program *const program)
{
    fclose(program->fptr);
    variable_array_free(program->variables);
}