#include "parser.h"

#include <string.h>

#include "lexer/lexer.h"

void *parse_tokens(struct token_list *token_list)
{
#define BUFFER 8
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
                strncpy(&parsed[cur_word * WORD_SIZE], O_CREATE_VAR, COMMAND_BYTES);
                strncpy(&parsed[cur_word * WORD_SIZE + 2], (char *)&token_list->tokens[1].type, 4);
            }
            else if (token_buffer[0]->type == PRINT)
            {
            }
            else if (token_buffer[1]->type == ASSIGN)
            {
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
    // free(token_buffer);

    strncpy(&parsed[cur_word * WORD_SIZE], EOP, COMMAND_BYTES);
    printf("TESTANDO4\n");
    return parsed;
#undef BUFFER_SIZE
}