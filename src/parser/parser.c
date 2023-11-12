#include "parser.h"

#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "variable_array.h"
#include "lexer/lexer.h"

#define BUFFER 8

struct parse_vars
{
    struct token **token_buffer;
    size_t token_buffer_count;
    struct variable_array *variable_array;
    size_t cur_word;
    char *parsed;
};

inline static void set_bytes(char *dst, char *src, size_t bytes)
{
    for (size_t i = 0; i < bytes; i++)
    {
        dst[i] = src[i];
    }
}

inline static void next_word(struct parse_vars *parse_vars)
{
    parse_vars->cur_word++;
    if (parse_vars->cur_word % BUFFER == 0)
    {
        parse_vars->parsed = realloc(parse_vars->parsed, sizeof(char) * WORD_SIZE * (parse_vars->cur_word + BUFFER));
    }
}

inline static void create_var(struct parse_vars *parse_vars)
{
    if (parse_vars->token_buffer_count < 3)
    {
        fprintf(stderr, "%s::%d: Statement has not enough arguments!\n", "filename", parse_vars->token_buffer[0]->line);
        return;
    }
    if (parse_vars->token_buffer[1]->type != NAME || parse_vars->token_buffer[2]->type != NAME)
    {
        fprintf(stderr, "%s::%d: Statement requires a type and a name!\n", "filename", parse_vars->token_buffer[0]->line);
        return;
    }
    struct variable variable =
        {
            .name = parse_vars->token_buffer[2]->text,
            .type = variable_type_from_string(parse_vars->token_buffer[1]->text),
        };
    if (variable.type == INVALID)
    {
        fprintf(stderr, "%s::%d: Type %s does not exists!\n", "filename", parse_vars->token_buffer[1]->line, parse_vars->token_buffer[1]->text);
        return;
    }
    if (variable_array_add(parse_vars->variable_array, variable) == -1)
    {
        fprintf(stderr, "%s::%d: Name %s already exists!\n", "filename", parse_vars->token_buffer[2]->line, parse_vars->token_buffer[2]->text);
        return;
    }
    int16_t two_byte_type = (int16_t)variable.type;
    set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE], C_CREATE_VAR, COMMAND_BYTES);
    set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES], (char *)&two_byte_type, TYPE_BYTES);
    next_word(parse_vars);
}

inline static void assign(struct parse_vars *parse_vars)
{
    if (parse_vars->token_buffer[0]->type != NAME)
    {
        fprintf(stderr, "%s::%d: Assign value is not a name!\n", __FILE__, __LINE__);
        return;
    }

    size_t index = variable_array_find(parse_vars->variable_array, parse_vars->token_buffer[0]->text);
    if (index < 0)
    {
        fprintf(stderr, "%s::%d: Variable %s does not exist!\n", "filename", parse_vars->token_buffer[0]->line, parse_vars->token_buffer[0]->text);
        return;
    }

    if (parse_vars->token_buffer[2]->type == NUMBER)
    {
        enum variable_type type;
        if (strchr(parse_vars->token_buffer[2]->text, '.') != NULL)
        {
            type = F32;
        }
        else
        {
            type = I32;
        }
        if (parse_vars->variable_array->data[index].type != type)
        {
            fprintf(stderr, "%d::%d: Invalid value assigned!\n", parse_vars->variable_array->data[index].type, type);
            return;
        }

        int16_t two_byte_type = (int16_t)type;

        // Assign
        set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE], C_ASSIGN, COMMAND_BYTES);
        set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES], (char *)&index, ADDRESS_BYTES);
        set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES + ADDRESS_BYTES], (char *)&two_byte_type, TYPE_BYTES);

        switch (type)
        {
        case I32:
            int32_t value_int = atoi(parse_vars->token_buffer[2]->text);
            set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES + ADDRESS_BYTES + TYPE_BYTES], (char *)&value_int, 4);
            break;

        case F32:
            float value_float = (float)atof(parse_vars->token_buffer[2]->text);
            set_bytes(&parse_vars->parsed[parse_vars->cur_word * WORD_SIZE + COMMAND_BYTES + ADDRESS_BYTES + TYPE_BYTES], (char *)&value_float, 4);
            break;

        default:
            break;
        }

        next_word(parse_vars);
    }
}

char *parse_tokens(struct token_list *token_list)
{
    struct parse_vars parse_vars =
        {
            .variable_array = variable_array_init(),
            .cur_word = 0,
            .parsed = malloc(sizeof(char) * WORD_SIZE * BUFFER),
            .token_buffer = malloc(sizeof(struct token *) * 16),
            .token_buffer_count = 0,
        };

    for (size_t i = 0; i < token_list->count; i++)
    {
        if (token_list->tokens[i].type == ENDLINE)
        {
            if (parse_vars.token_buffer[0]->type == CREATE_VAR)
            {
                create_var(&parse_vars);
            }
            else if (parse_vars.token_buffer[0]->type == PRINT)
            {
                if (parse_vars.token_buffer_count < 4)
                {
                    fprintf(stderr, "%s::%d: Not enough tokens!\n", __FILE__, __LINE__);
                    return NULL;
                }
                if (parse_vars.token_buffer[1]->type != CALLER_START)
                {
                    fprintf(stderr, "%s::%d: Caller Start unavailable!\n", __FILE__, __LINE__);
                    return NULL;
                }
                if (parse_vars.token_buffer[parse_vars.token_buffer_count - 1]->type != CALLER_END)
                {
                    fprintf(stderr, "%s::%d: Caller end missing!\n", __FILE__, __LINE__);
                    return NULL;
                }

                set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE], C_PRINT, COMMAND_BYTES);
                size_t index = variable_array_find(parse_vars.variable_array, parse_vars.token_buffer[2]->text);
                if (index < 0)
                {
                    fprintf(stderr, "%s::%d: Variable %s does not exist!\n", __FILE__, __LINE__, parse_vars.token_buffer[2]->text);
                }

                int16_t two_byte_type = parse_vars.variable_array->data[index].type;
                set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE + COMMAND_BYTES], (char *)&index, ADDRESS_BYTES);
                set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE + COMMAND_BYTES + ADDRESS_BYTES], (char *)&two_byte_type, TYPE_BYTES);
                next_word(&parse_vars);
            }
            else if (parse_vars.token_buffer[1]->type == ASSIGN)
            {
                assign(&parse_vars);
            }
            else if (parse_vars.token_buffer[1]->type == ADD)
            {
                set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE], C_OPERATION, COMMAND_BYTES);
                set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE + COMMAND_BYTES], O_ADD, OPERATION_BYTES);
                next_word(&parse_vars);
            }
            else
            {
                fprintf(stderr, "%s::%d: Token does not exist!\n", __FILE__, __LINE__);
            }

            parse_vars.token_buffer_count = 0;
        }
        else
        {
            parse_vars.token_buffer[parse_vars.token_buffer_count] = &token_list->tokens[i];
            parse_vars.token_buffer_count++;
        }
    }
    free(parse_vars.token_buffer);
    variable_array_free(parse_vars.variable_array);

    set_bytes(&parse_vars.parsed[parse_vars.cur_word * WORD_SIZE], C_EOP, COMMAND_BYTES);
    return parse_vars.parsed;
}
#undef BUFFER_SIZE