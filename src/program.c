#include "program.h"

#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>

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

    program->variables = NULL;
    program->variable_count = 0;
    program->cur_line = 0;
    return program;
}

/// @brief Adds variable to program
/// @param program
/// @param variable_type
static void program_add_var(struct program *const program, enum variable_type variable_type)
{
    void *data = NULL;
    switch (variable_type)
    {
    case I32:
        data = malloc(sizeof(int32_t));
        *((int32_t *)data) = 0;
        break;
    case F32:
        data = malloc(sizeof(float));
        *((float *)data) = 0.0;
        break;
    case INVALID:
        printf("invalid\n");
        return;
    default:
        printf("invalid\n");
        return;
    }

    program->variable_count++;
    program->variables = realloc(program->variables, sizeof(void *) * program->variable_count);
    program->variables[program->variable_count - 1] = data;
}

int program_set_var(const struct program *const program, const char *const variable_name, const struct variable assign)
{

    // program_print_error(program, "Trying to assign %s to %s:%s!\n", variable_type_to_string(assign.type), variable_type_to_string(variable->type), variable->name);
    return -1;
}

inline static int compare_bytes(char *x, char *y, size_t bytes)
{
    for (size_t i = 0; i < bytes; i++)
    {
        if (x[i] ^ y[i])
        {
            return 0;
        }
    }

    return 1;
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

    for (program->cur_line = 0; strncmp(&parsed_program[program->cur_line * WORD_SIZE], C_EOP, COMMAND_BYTES) == 0; program->cur_line++)
    {
        if (compare_bytes(&parsed_program[program->cur_line * WORD_SIZE], C_CREATE_VAR, COMMAND_BYTES))
        {
            uint16_t *var_type = (uint16_t *)&parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES];
            program_add_var(program, *var_type);
        }
        else if (compare_bytes(&parsed_program[program->cur_line * WORD_SIZE], C_PRINT, COMMAND_BYTES))
        {
            uint32_t *index = (uint32_t *)&parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES];
            uint16_t *type = (uint16_t *)&parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES + ADDRESS_BYTES];
            switch (*type)
            {
            case I32:
                printf("%d\n", *(int32_t *)(program->variables[*index]));
                break;
            case F32:
                printf("%f\n", *(float *)(program->variables[*index]));
                break;
            case INVALID:
                printf("Invalid variable type - Printing\n");
                break;
            default:
                printf("Invalid variable type - Printing\n");
                break;
            }
        }
        else if (compare_bytes(&parsed_program[program->cur_line * WORD_SIZE], C_ASSIGN, COMMAND_BYTES))
        {
            int *assign_index = (int *)&parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES];
            uint16_t *type = (uint16_t *)&parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES + ADDRESS_BYTES];
            void *value = &parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES + ADDRESS_BYTES + TYPE_BYTES];
            switch (*type)
            {
            case I32:
                *(int32_t *)program->variables[*assign_index] = *(int32_t *)value;
                break;
            case F32:
                *(float *)program->variables[*assign_index] = *(float *)value;
                break;
            case INVALID:
                printf("Invalid variable type - Assign\n");
                break;
            default:
                printf("Invalid variable type - Assign\n");
                break;
            }
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
    // variable_array_free(program->variables);
}