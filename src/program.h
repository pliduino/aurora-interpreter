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
    COMPILE = 1 << 2,
    RUN_COMPILED = 1 << 3,
    TRANSPILE_C = 1 << 4,
};

struct program
{
    FILE *fptr;
    const char *file_path;
    void **variables;
    size_t variable_count;
    long long int cur_line;
    char options;
};

struct program *program_init(const char *const file_path);

int program_run(struct program *const program);

void program_close(const struct program *const program);

#endif