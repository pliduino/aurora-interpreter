#ifndef AURORA_PROGRAM_H
#define AURORA_PROGRAM_H

#include <stdint.h>
#include <stdio.h>

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
    void *stack;
    struct stack_offset *stack_offset;
    long long int cur_line;
    char options; // Can be expanded to 8 bytes with no cost if needed (padding)
};

struct stack_offset
{
    uint32_t value;
    struct stack_offset *previous_offset;
};

struct program *program_init(const char *const file_path);

int program_run(struct program *const program);

void program_close(struct program *const program);

#endif