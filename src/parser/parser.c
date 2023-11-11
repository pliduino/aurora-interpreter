#include "parser.h"

#include <string.h>

#include "variable_array.h"
#include "lexer/lexer.h"

inline static void set_bytes(char *dst, char *src, size_t bytes)
{
    for (size_t i = 0; i < bytes; i++)
    {
        dst[i] = src[i];
    }
}

char *parse_tokens(struct token_list *token_list)
{
#define BUFFER 8
    struct variable_array *variable_array = variable_array_init();
    char *parsed = malloc(sizeof(char) * WORD_SIZE * BUFFER);
    size_t cur_word = 0;

    struct token **token_buffer = malloc(sizeof(struct token *) * 16);
    size_t token_buffer_count = 0;
    for (size_t i = 0; i < token_list->count; i++)
    {
        if (token_list->tokens[i].type == ENDLINE)
        {
            if (token_buffer[0]->type == CREATE_VAR)
            {
                if (token_buffer_count != 3)
                {
                    // program_print_error(program, "Invalid var format!\n");
                    free(token_buffer);
                    return NULL;
                }
                if (token_buffer[1]->type != NAME || token_buffer[2]->type != NAME)
                {
                    // program_print_error(program, "Invalid var format!\n");
                    free(token_buffer);
                    return NULL;
                }
                struct variable variable =
                    {
                        .name = token_buffer[2]->text,
                        .type = variable_type_from_string(token_buffer[1]->text),
                    };
                variable_array_add(variable_array, variable);

                set_bytes(&parsed[cur_word * WORD_SIZE], C_CREATE_VAR, COMMAND_BYTES);
                set_bytes(&parsed[cur_word * WORD_SIZE + COMMAND_BYTES], (char *)&variable.type, 4);
            }
            else if (token_buffer[0]->type == PRINT)
            {
                set_bytes(&parsed[cur_word * WORD_SIZE], C_PRINT, COMMAND_BYTES);
                int index = variable_array_find(variable_array, token_buffer[1]->text);
                if (index < 0)
                {
                    fprintf(stderr, "%s:%d - Variable does not exist!\n", __FILE__, __LINE__);
                }

                set_bytes(&parsed[cur_word * WORD_SIZE + COMMAND_BYTES], (char *)&index, 4);
                set_bytes(&parsed[cur_word * WORD_SIZE + COMMAND_BYTES + 4], (char *)&variable_array->data[index].type, 4);
            }
            else if (token_buffer[1]->type == ASSIGN)
            {
            }
            else if (token_buffer[1]->type == ADD)
            {
                set_bytes(&parsed[cur_word * WORD_SIZE], C_OPERATION, COMMAND_BYTES);
                set_bytes(&parsed[cur_word * WORD_SIZE + COMMAND_BYTES], O_ADD, OPERATION_BYTES);
            }
            else
            {
                fprintf(stderr, "%s:%d - Token does not exist!\n", __FILE__, __LINE__);
            }

            token_buffer_count = 0;
            cur_word++;
            if (cur_word % BUFFER == 0)
            {
                parsed = realloc(parsed, sizeof(char) * WORD_SIZE * (cur_word + BUFFER));
            }
        }
        else
        {
            token_buffer[token_buffer_count] = &token_list->tokens[i];
            token_buffer_count++;
        }
    }
    free(token_buffer);
    variable_array_free(variable_array);

    set_bytes(&parsed[cur_word * WORD_SIZE], C_EOP, COMMAND_BYTES);
    return parsed;
#undef BUFFER_SIZE
}