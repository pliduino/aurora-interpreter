#include "lexer.h"

#include <stdio.h>
#include <string.h>

char **separate_string(const char *const string, size_t *word_count)
{
#define BUFFER_SIZE 4
    int count = 0;
    char **words = malloc(sizeof(char **) * BUFFER_SIZE);
    char **temp = NULL;
    (*word_count) = 0;
    for (int i = 0; i < strlen(string); i++)
    {
        if (string[i] == ' ' || string[i] == '\n' || string[i] == '\r')
        {
            if (count > 0)
            {
                words[*word_count] = malloc(sizeof(char) * (count + 1));
                strncpy(words[*word_count], &string[i - count], count);
                words[*word_count][count] = '\0';
                count = 0;

                (*word_count)++;
                if (*word_count % BUFFER_SIZE == 0)
                {
                    temp = realloc(words, *word_count + BUFFER_SIZE);
                    if (temp == NULL)
                    {
                        return NULL;
                    }
                    words = temp;
                }
            }
            continue;
        }
        count++;
    }
    if (count > 0)
    {
        words[*word_count] = malloc(sizeof(char) * (count + 1));
        strcpy(words[*word_count], &string[strlen(string) - count]);
        words[*word_count][count] = '\0';
        count = 0;

        (*word_count)++;
    }

    return words;
#undef BUFFER_SIZE
}

struct token_list *lex_line(char *line)
{
    struct token_list *token_list = token_list_init();
    size_t word_count;
    char **words = separate_string(line, &word_count);
    if (words == NULL)
    {
        token_list_destroy(token_list);
        return NULL;
    }

    token_list->count = 0;
    for (size_t i = 0; i < word_count; i++)
    {
        if (strcmp(words[i], "var") == 0)
        {
            struct token token = {.text = words[i], .type = CREATE_VAR};
            token_list_push_back(token_list, token);
        }
        else if (strcmp(words[i], "+") == 0)
        {
            struct token token = {.text = words[i], .type = ADD};
            token_list_push_back(token_list, token);
        }
        else if (strcmp(words[i], "=") == 0)
        {
            struct token token = {.text = words[i], .type = ASSIGN};
            token_list_push_back(token_list, token);
        }
        else if (words[i][0] >= '0' && words[i][0] <= '9')
        {
            struct token token = {.text = words[i], .type = NUMBER};
            token_list_push_back(token_list, token);
        }
        else if (strcmp(words[i], "print") == 0)
        {
            struct token token = {.text = words[i], .type = PRINT};
            token_list_push_back(token_list, token);
        }
        else
        {
            struct token token = {.text = words[i], .type = NAME};
            token_list_push_back(token_list, token);
        }
    }

    return token_list;
}

void token_list_push_back(struct token_list *const token_list, const struct token token)
{
    token_list->count++;
    token_list->tokens = realloc(token_list->tokens, sizeof(struct token_list) * (token_list->count));
    token_list->tokens[token_list->count - 1] = token;
}

void token_list_destroy(struct token_list *const token_list)
{
    // free(token_list->tokens);
    // free(token_list);
}

struct token_list *token_list_init()
{
    struct token_list *token_list = malloc(sizeof(token_list));
    token_list->tokens = NULL;
    token_list->count = 0;
    return token_list;
}