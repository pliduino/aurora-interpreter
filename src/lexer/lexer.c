#include "lexer.h"

#include <stdio.h>
#include <string.h>

static int chrcmp(const char *const cmp_c, const char c)
{
    for (size_t i = 0; cmp_c[i] != '\0'; i++)
    {
        if (c == cmp_c[i])
        {
            return 1;
        }
    }
    return 0;
}

char **separate_file(FILE *const file, size_t *word_count)
{
#define WORD_BUFFER_SIZE 4
#define BUFFER_SIZE 65536
    int count = 0;
    char *buffer = malloc(sizeof(char) * BUFFER_SIZE);
    char **words = malloc(sizeof(char *) * WORD_BUFFER_SIZE);
    char **temp = NULL;
    (*word_count) = 0;

    size_t buffer_size;
    // FIX if buffer breaks in the middle of a word (move file back by count?)
    while ((buffer_size = fread(buffer, sizeof(char), BUFFER_SIZE, file)) != 0)
    {
        for (int i = 0; i < buffer_size; i++)
        {
            if (chrcmp(";()[]{}+-*/=<>&,", buffer[i]))
            {
                // Adds word before symbol
                if (count > 0)
                {
                    words[*word_count] = malloc(sizeof(char) * (count + 1));
                    strncpy(words[*word_count], &buffer[i - count], count);
                    words[*word_count][count] = '\0';
                    count = 0;

                    (*word_count)++;
                    if (*word_count % WORD_BUFFER_SIZE == 0)
                    {
                        temp = realloc(words, sizeof(char *) * (*word_count + WORD_BUFFER_SIZE));
                        if (temp == NULL)
                        {
                            free(buffer);
                            return NULL;
                        }
                        words = temp;
                    }
                }

                // Adds symbol
                words[*word_count] = malloc(sizeof(char) * 2);
                words[*word_count][0] = buffer[i];
                words[*word_count][1] = '\0';

                (*word_count)++;
                if (*word_count % WORD_BUFFER_SIZE == 0)
                {
                    temp = realloc(words, sizeof(char *) * (*word_count + WORD_BUFFER_SIZE));
                    if (temp == NULL)
                    {
                        free(buffer);
                        return NULL;
                    }
                    words = temp;
                }
            }
            else if (chrcmp(" \n\r\t", buffer[i]))
            {
                if (count > 0)
                {
                    words[*word_count] = malloc(sizeof(char) * (count + 1));
                    strncpy(words[*word_count], &buffer[i - count], count);
                    words[*word_count][count] = '\0';
                    count = 0;

                    (*word_count)++;
                    if (*word_count % WORD_BUFFER_SIZE == 0)
                    {
                        temp = realloc(words, sizeof(char *) * (*word_count + WORD_BUFFER_SIZE));
                        if (temp == NULL)
                        {
                            free(buffer);
                            return NULL;
                        }
                        words = temp;
                    }
                }
            }
            else
            {
                count++;
            }
        }
    }

    if (count > 0)
    {
        words[*word_count] = malloc(sizeof(char) * (count + 1));
        strcpy(words[*word_count], &buffer[strlen(buffer) - count]);
        words[*word_count][count] = '\0';
        count = 0;

        (*word_count)++;
    }

    free(buffer);
    return words;
#undef WORD_BUFFER_SIZE
#undef BUFFER_SIZE
}

struct token_list *lex_file(FILE *const file)
{
    struct token_list *token_list = token_list_init();
    size_t word_count;
    char **words = separate_file(file, &word_count);
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
        else if (words[i][0] == '+')
        {
            struct token token = {.text = words[i], .type = ADD};
            token_list_push_back(token_list, token);
        }
        else if (words[i][0] == '-')
        {
            struct token token = {.text = words[i], .type = SUBTRACT};
            token_list_push_back(token_list, token);
        }
        else if (words[i][0] == '*')
        {
            struct token token = {.text = words[i], .type = MULTIPLY};
            token_list_push_back(token_list, token);
        }
        else if (words[i][0] == '/')
        {
            struct token token = {.text = words[i], .type = DIVIDE};
            token_list_push_back(token_list, token);
        }
        else if (words[i][0] == '=')
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
        else if (words[i][0] == ';')
        {
            struct token token = {.text = words[i], .type = ENDLINE};
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