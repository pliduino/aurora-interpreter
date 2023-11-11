#ifndef AURORA_PROGRAM_H
#define AURORA_PROGRAM_H

#include <stdio.h>

#include "variable_array.h"

struct token_list;

enum program_options
{
    EMPTY = 0,
    VERBOSE = 1 << 0,
    STRICT = 1 << 1,
};

struct program
{
    FILE *fptr;
    const char *file_path;
    struct variable_array *variables;
    long long int cur_line;
    char options;
};

int handle_token_list(struct program *program, struct token_list *token_list);

struct program *program_init(const char *const file_path);

void program_run(struct program *const program);

void program_close(const struct program *const program);

#endif