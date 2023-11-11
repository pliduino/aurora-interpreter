#include "lexer.h"

#include <stdio.h>
#include <string.h>

char **separate_string(const char *const string, size_t *word_count)
{
    int count = 0;
    char **words = NULL;
    char **temp = NULL;

    (*word_count) = 0;
    for (size_t i = 0; i < strlen(string); i++)
    {
        if (string[i] == ' ' || string[i] == '\n' || string[i] == '\r' || string[i] == EOF || string[i] == '\0')
        {
            if (count > 0)
            {
                (*word_count)++;
                temp = realloc(words, sizeof(char **) * (*word_count));
                if (temp == NULL)
                {
                    fprintf(stderr, "%s:%d - Realloc crashed!", __FILE__, __LINE__);
                    free(words);
                    return NULL;
                }
                words = temp;
                size_t string_size = sizeof(char) * (count + 1);

                words[(*word_count) - 1] = malloc(string_size);
                strncpy_s(words[(*word_count) - 1], sizeof(char) * (count + 1), &string[i - count], count);
                words[(*word_count) - 1][count] = '\0';

                count = 0;
            }
            continue;
        }
        count++;
    }
    if (count > 0)
    {
        (*word_count)++;
        temp = realloc(words, sizeof(char **) * (*word_count * 7));
        if (temp == NULL)
        {
            fprintf(stderr, "%s:%d - Realloc crashed!", __FILE__, __LINE__);
            free(words);
            return NULL;
        }
        words = temp;
        size_t string_size = sizeof(char) * (count + 1);
        words[(*word_count) - 1] = malloc(string_size);
        strncpy_s(&*words[(*word_count) - 1], sizeof(char) * (count + 1), &string[strlen(string) - count], count);
        words[(*word_count) - 1][count] = '\0';
        count = 0;
    }

    return words;
}

struct token_list *lex_line(char *line)
{
    struct token_list *token_list = token_list_init();
    token_list->count = 0;
    size_t word_count;
    char **words = separate_string(line, &word_count);
    if (words == NULL)
    {
        token_list_destroy(token_list);
        return NULL;
    }

    for (size_t i = 0; i < word_count; i++)
    {
        printf("word %zd -- %s\n", i, words[i]);
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
    free(token_list->tokens);
    free(token_list);
}

struct token_list *token_list_init()
{
    struct token_list *token_list = malloc(sizeof(token_list));
    token_list->tokens = NULL;
    token_list->count = 0;
    return token_list;
}