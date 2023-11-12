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
    case I8:
        data = malloc(sizeof(int8_t));
        *((int8_t *)data) = 0;
        break;
    case I16:
        data = malloc(sizeof(int16_t));
        *((int16_t *)data) = 0;
        break;
    case I32:
        data = malloc(sizeof(int32_t));
        *((int32_t *)data) = 0;
        break;
    case I64:
        data = malloc(sizeof(int64_t));
        *((int64_t *)data) = 0;
        break;
    case F32:
        data = malloc(sizeof(float));
        *((float *)data) = 0.0;
        break;
    case F64:
        data = malloc(sizeof(double));
        *((double *)data) = 0.0;
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

static inline int compare_bytes(char *x, char *y, size_t bytes)
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

int program_run(struct program *const program)
{
    clock_t exec_time;
    char *parsed_program;
    FILE *transpile;
    uint32_t transpiler_temp = 0;
    char transpile_buffer[1024];
    exec_time = clock();

    // Runs pre-compiled bytecode
    if (program->options & RUN_COMPILED)
    {
        fseek(program->fptr, 0, SEEK_END);
        long file_size = ftell(program->fptr);
        fseek(program->fptr, 0, SEEK_SET);

        parsed_program = malloc(file_size + 1);
        fread(parsed_program, file_size, 1, program->fptr);
    }
    // Parses aurora file
    else
    {
        struct token_list *token_list = lex_file(program->fptr);
        parsed_program = parse_tokens(token_list);
        token_list_destroy(token_list);
        if (parsed_program == NULL)
        {
            fprintf(stderr, "%s:%d - Error while parsing!\n", __FILE__, __LINE__);
            return -1;
        }
    }

    if (program->options & COMPILE)
    {
        FILE *fp = fopen("test.arc", "w");
        if (fp == NULL)
        {
            fprintf(stderr, "Could not open %s!\n", "test.arc");
            return -1;
        }
        for (program->cur_line = 0; strncmp(&parsed_program[program->cur_line * WORD_SIZE], C_EOP, COMMAND_BYTES) == 0; program->cur_line++)
        {
            fwrite(&parsed_program[program->cur_line * WORD_SIZE], WORD_SIZE, 1, fp);
        }
        fwrite(&parsed_program[program->cur_line * WORD_SIZE], WORD_SIZE, 1, fp);
        return 0;
    }

    if (program->options & TRANSPILE_C)
    {
        transpile = fopen("transpiled.c", "w");
        if (transpile == NULL)
        {
            fprintf(stderr, "Could not open %s!\n", "transpiled.c");
            return -1;
        }

        fputs("#include <stdint.h>\n"
              "#include <stdio.h>\n"
              "int main(){\n",
              transpile);
    }

    for (program->cur_line = 0; strncmp(&parsed_program[program->cur_line * WORD_SIZE], C_EOP, COMMAND_BYTES) == 0; program->cur_line++)
    {
        if (compare_bytes(&parsed_program[program->cur_line * WORD_SIZE], C_CREATE_VAR, COMMAND_BYTES))
        {
            uint16_t *var_type = (uint16_t *)&parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES];
            if (program->options & TRANSPILE_C)
            {
                char *type_string;
                switch (*var_type)
                {
                case I8:
                    type_string = "int8_t";
                    break;
                case I16:
                    type_string = "int16_t";
                    break;
                case I32:
                    type_string = "int32_t";
                    break;
                case I64:
                    type_string = "int64_t";
                    break;
                case F32:
                    type_string = "float";
                    break;
                case F64:
                    type_string = "double";
                    break;
                default:
                    break;
                }
                sprintf(transpile_buffer, "%s var_%d;\n", type_string, transpiler_temp++);
                fputs(transpile_buffer, transpile);
            }
            else
            {
                program_add_var(program, *var_type);
            }
        }
        else if (compare_bytes(&parsed_program[program->cur_line * WORD_SIZE], C_PRINT, COMMAND_BYTES))
        {
            uint32_t *index = (uint32_t *)&parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES];
            uint16_t *type = (uint16_t *)&parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES + ADDRESS_BYTES];
            if (program->options & TRANSPILE_C)
            {
                char *type_string;
                char *conversion_string = "";
                switch (*type)
                {
                case I8:
                    type_string = "%d";
                    char *conversion_string = "(int)";
                case I16:
                    type_string = "%hd";
                case I32:
                    type_string = "%d";
                    break;
                case I64:
                    type_string = "%lld";
                    break;
                case F32:
                    type_string = "%f";
                case F64:
                    type_string = "%lf";
                default:
                    break;
                }
                sprintf(transpile_buffer, "printf(\"%s\\n\", %svar_%u);\n", type_string, conversion_string, *index);
                fputs(transpile_buffer, transpile);
            }
            else
            {
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
        }
        else if (compare_bytes(&parsed_program[program->cur_line * WORD_SIZE], C_ASSIGN, COMMAND_BYTES))
        {
            int *assign_index = (int *)&parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES];
            uint16_t *type = (uint16_t *)&parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES + ADDRESS_BYTES];
            void *value = &parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES + ADDRESS_BYTES + TYPE_BYTES];

            if (program->options & TRANSPILE_C)
            {
                switch (*type)
                {
                case I8:
                    sprintf(transpile_buffer, "var_%d = %d;", *assign_index, (int)*(int8_t *)value);
                    break;
                case I16:
                    sprintf(transpile_buffer, "var_%d = %hd;", *assign_index, *(int16_t *)value);
                    break;
                case I32:
                    sprintf(transpile_buffer, "var_%d = %d;", *assign_index, *(int32_t *)value);
                    break;
                case I64:
                    sprintf(transpile_buffer, "var_%d = %lld;", *assign_index, *(int64_t *)value);
                    break;
                case F32:
                    sprintf(transpile_buffer, "var_%d = %f;", *assign_index, *(float *)value);
                case F64:
                    sprintf(transpile_buffer, "var_%d = %lf;", *assign_index, *(double *)value);
                default:
                    break;
                }
                fputs(transpile_buffer, transpile);
            }
            else
            {
                switch (*type)
                {
                case I8:
                    *(int8_t *)program->variables[*assign_index] = *(int8_t *)value;
                    break;
                case I16:
                    *(int16_t *)program->variables[*assign_index] = *(int16_t *)value;
                    break;
                case I32:
                    *(int32_t *)program->variables[*assign_index] = *(int32_t *)value;
                    break;
                case I64:
                    *(int64_t *)program->variables[*assign_index] = *(int64_t *)value;
                    break;
                case F32:
                    *(float *)program->variables[*assign_index] = *(float *)value;
                    break;
                case F64:
                    *(double *)program->variables[*assign_index] = *(double *)value;
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
    }
    if (program->options & TRANSPILE_C)
    {
        fputs("}\n", transpile);
    }

    free(parsed_program);

    exec_time = clock() - exec_time;
    double time_taken = (((double)exec_time) / CLOCKS_PER_SEC) * 1000;
    printf("\nExecution time: %.0fms\n", time_taken);
    return 0;
}

/// @brief Closes file and frees variables
/// @param program
void program_close(const struct program *const program)
{
    fclose(program->fptr);
    for (size_t i = 0; i < program->variable_count; i++)
    {
        free(program->variables[i]);
    }
    free(program->variables);
}