#ifndef AURORA_LEXER_H
#define AURORA_LEXER_H

#include <malloc.h>
#include <stdio.h>

enum token_type
{
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    CALLER_START,
    CALLER_END,
    BLOCK_START,
    BLOCK_END,
    CREATE_VAR,
    ASSIGN,
    NUMBER,
    NAME,
    PRINT,
    ENDLINE,
    FUNCTION,
    SEPARATOR,
};

struct token
{
    int line;
    enum token_type type;
    char *text;
};

struct token_list
{
    struct token *tokens;
    size_t count;
};

struct token_list *token_list_init(void);

void token_list_push_back(struct token_list *const token_list, const struct token token);

void token_list_destroy(struct token_list *const token_list);

struct token_list *lex_file(FILE *const string);

#endif