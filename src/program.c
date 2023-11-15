#include "program.h"

#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "parser/variable_array.h"
#include "helpers.h"

#define KILOBYTE 1000
#define MEGABYTE KILOBYTE * 1000

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
    program->fptr = fopen(file_path, "r");

    if (program->fptr == NULL)
    {
        fprintf(stderr, "%s:%d - File does not exist!\n", __FILE__, __LINE__);
        return NULL;
    }

    program->stack = malloc(1 * MEGABYTE);
    program->cur_line = 0;
    return program;
}

/// @brief Adds variable to program
/// @param program
/// @param variable_type
static void program_add_var(struct program *const program, enum variable_type variable_type, size_t position)
{
    memset((char *)program->stack + position, 0, get_size_of_type(variable_type));
}

int program_run(struct program *const program)
{
    clock_t exec_time;
    char *parsed_program;
    FILE *transpile;
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
        parsed_program = parse_tokens(token_list, 0);
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
              "#include <stdbool.h>\n"
              "int main(){\n",
              transpile);
    }

    for (program->cur_line = 0; compare_bytes(&parsed_program[program->cur_line * WORD_SIZE], C_EOP, COMMAND_BYTES) == 0; program->cur_line++)
    {
        if (compare_bytes(&parsed_program[program->cur_line * WORD_SIZE], C_CREATE_VAR, COMMAND_BYTES))
        {
            uint32_t *position = (uint32_t *)&parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES];
            uint16_t *var_type = (uint16_t *)&parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES + ADDRESS_BYTES];
            if (program->options & TRANSPILE_C)
            {
                char *type_string = "";
                switch (*var_type)
                {
                case BOOL:
                    type_string = "bool";
                    break;
                case CHAR:
                    type_string = "char";
                    break;
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
                case U8:
                    type_string = "uint8_t";
                    break;
                case U16:
                    type_string = "uint16_t";
                    break;
                case U32:
                    type_string = "uint32_t";
                    break;
                case U64:
                    type_string = "uint64_t";
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
                sprintf(transpile_buffer, "%s var_%d;\n", type_string, *position);
                fputs(transpile_buffer, transpile);
            }
            else
            {
                program_add_var(program, *var_type, *position);
            }
        }
        else if (compare_bytes(&parsed_program[program->cur_line * WORD_SIZE], C_PRINT, COMMAND_BYTES))
        {
            uint32_t *index = (uint32_t *)&parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES];
            uint16_t *type = (uint16_t *)&parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES + ADDRESS_BYTES];
            if (program->options & TRANSPILE_C)
            {
                switch (*type)
                {
                case BOOL:
                    sprintf(transpile_buffer, "if(var_%u){printf(\"True\n\")} else {printf(\"False\n\", );\n}", *index);
                    break;
                case CHAR:
                    sprintf(transpile_buffer, "printf(\"%%c\\n\", var_%u);\n", *index);
                    break;
                case I8:
                    sprintf(transpile_buffer, "printf(\"%%d\\n\", var_%u);\n", *index);
                    break;
                case I16:
                    sprintf(transpile_buffer, "printf(\"%%hd\\n\", var_%u);\n", *index);
                    break;
                case I32:
                    sprintf(transpile_buffer, "printf(\"%%d\\n\", var_%u);\n", *index);
                    break;
                case I64:
                    sprintf(transpile_buffer, "printf(\"%%lld\\n\", var_%u);\n", *index);
                    break;
                case U8:
                    sprintf(transpile_buffer, "printf(\"%%u\\n\", var_%u);\n", *index);
                    break;
                case U16:
                    sprintf(transpile_buffer, "printf(\"%%hu\\n\", var_%u);\n", *index);
                    break;
                case U32:
                    sprintf(transpile_buffer, "printf(\"%%u\\n\", var_%u);\n", *index);
                    break;
                case U64:
                    sprintf(transpile_buffer, "printf(\"%%llu\\n\", var_%u);\n", *index);
                    break;
                case F32:
                    sprintf(transpile_buffer, "printf(\"%%f\\n\", var_%u);\n", *index);
                    break;
                case F64:
                    sprintf(transpile_buffer, "printf(\"%%lf\\n\", var_%u);\n", *index);
                    break;
                default:
                    break;
                }
                fputs(transpile_buffer, transpile);
            }
            else
            {
                switch (*type)
                {
                case BOOL:
                    if (*(char *)((char *)program->stack + *index))
                    {
                        printf("True\n");
                    }
                    else
                    {
                        printf("False\n");
                    }
                    break;
                case CHAR:
                    printf("%c\n", *(char *)((char *)program->stack + *index));
                    break;
                case I8:
                    printf("%d\n", (int32_t)(*(int8_t *)((char *)program->stack + *index)));
                    break;
                case I16:
                    printf("%hd\n", *(int16_t *)((char *)program->stack + *index));
                    break;
                case I32:
                    printf("%d\n", *(int32_t *)((char *)program->stack + *index));
                    break;
                case I64:
                    printf("%lld\n", *(int64_t *)((char *)program->stack + *index));
                    break;
                case U8:
                    printf("%u\n", (uint32_t)(*(uint8_t *)((char *)program->stack + *index)));
                    break;
                case U16:
                    printf("%hu\n", *(uint16_t *)((char *)program->stack + *index));
                    break;
                case U32:
                    printf("%u\n", *(uint32_t *)((char *)program->stack + *index));
                    break;
                case U64:
                    printf("%llu\n", *(uint64_t *)((char *)program->stack + *index));
                    break;
                case F32:
                    printf("%f\n", *(float *)((char *)program->stack + *index));
                    break;
                case F64:
                    printf("%lf\n", *(double *)((char *)program->stack + *index));
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
            int *assign_pointer = (int *)&parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES];
            uint16_t *type = (uint16_t *)&parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES + ADDRESS_BYTES];
            void *value = &parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES + ADDRESS_BYTES + TYPE_BYTES];

            if (program->options & TRANSPILE_C)
            {
                switch (*type)
                {
                case BOOL:
                    sprintf(transpile_buffer, "var_%d = %d;", *assign_pointer, (int32_t) * (int8_t *)value);
                    break;
                case CHAR:
                    sprintf(transpile_buffer, "var_%d = '%c';", *assign_pointer, *(char *)value);
                    break;
                case I8:
                    sprintf(transpile_buffer, "var_%d = %d;", *assign_pointer, (int32_t) * (int8_t *)value);
                    break;
                case I16:
                    sprintf(transpile_buffer, "var_%d = %hd;", *assign_pointer, *(int16_t *)value);
                    break;
                case I32:
                    sprintf(transpile_buffer, "var_%d = %d;", *assign_pointer, *(int32_t *)value);
                    break;
                case I64:
                    sprintf(transpile_buffer, "var_%d = %lld;", *assign_pointer, *(int64_t *)value);
                    break;
                case U8:
                    sprintf(transpile_buffer, "var_%d = %u;", *assign_pointer, (uint32_t) * (uint8_t *)value);
                    break;
                case U16:
                    sprintf(transpile_buffer, "var_%d = %hu;", *assign_pointer, *(uint16_t *)value);
                    break;
                case U32:
                    sprintf(transpile_buffer, "var_%d = %u;", *assign_pointer, *(uint32_t *)value);
                    break;
                case U64:
                    sprintf(transpile_buffer, "var_%d = %llu;", *assign_pointer, *(uint64_t *)value);
                    break;
                case F32:
                    sprintf(transpile_buffer, "var_%d = %f;", *assign_pointer, *(float *)value);
                case F64:
                    sprintf(transpile_buffer, "var_%d = %lf;", *assign_pointer, *(double *)value);
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
                    *(int8_t *)((char *)program->stack + *assign_pointer) = *(int8_t *)value;
                    break;
                case I16:
                    *(int16_t *)((char *)program->stack + *assign_pointer) = *(int16_t *)value;
                    break;
                case I32:
                    *(int32_t *)((char *)program->stack + *assign_pointer) = *(int32_t *)value;
                    break;
                case I64:
                    *(int64_t *)((char *)program->stack + *assign_pointer) = *(int64_t *)value;
                    break;
                case F32:
                    *(float *)((char *)program->stack + *assign_pointer) = *(float *)value;
                    break;
                case F64:
                    *(double *)((char *)program->stack + *assign_pointer) = *(double *)value;
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
        else if (compare_bytes(&parsed_program[program->cur_line * WORD_SIZE], C_JUMP, COMMAND_BYTES))
        {
            char jump_type = *(char *)&parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES];
            uint32_t line = *(uint32_t *)&parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES + 1];
            if (jump_type == 1) // Jump as ref
            {
                uint32_t *value_address = (uint32_t *)((char *)(program->stack) + line);
                program->cur_line = *(uint32_t *)((char *)(program->stack) + line);
            }
            else // Jump as value
            {
                program->cur_line = line;
            }
        }
        else if (compare_bytes(&parsed_program[program->cur_line * WORD_SIZE], C_SETBLOCK, COMMAND_BYTES))
        {
            uint32_t *address = (uint32_t *)&parsed_program[program->cur_line * WORD_SIZE + COMMAND_BYTES];
            program->stack = (char *)program->stack + *address;
            program->stack_offset = *address;
        }
        else if (compare_bytes(&parsed_program[program->cur_line * WORD_SIZE], C_BACKBLOCK, COMMAND_BYTES))
        {
            program->stack = (char *)program->stack - program->stack_offset;
            program->stack_offset = 0;
        }
    }
    if (program->options & TRANSPILE_C)
    {
        fputs("}\n", transpile);
    }

    // free(parsed_program);

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
    free(program->stack);
}