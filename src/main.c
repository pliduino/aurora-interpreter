#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <string.h>

#include "program/program.h"

int main(int argc, char const *argv[])
{
    static const char usestr[] =
        "[-v verbose][-s strict][-c compile][-x execute][-t transpile]\n"
        "Other options: \n"
        "NB: -h gives more help!\n";

    static const char fullhelp[] =
        "\nOption summary:\n"
        "   -v --verbose   - Prints line being parsed\n"
        "   -s --strict    - Stops program in any error\n"
        "   -c --compile   - Pre-compiles the program into bytecode\n"
        "   -x --execute   - Runs pre-compiled bytecode\n"
        "   -t --transpile - Transpiles code to C"
        "   -h --help      - Prints this message\n";

    if (argc < 2)
    {
        fprintf(stderr, "%s:%d - No file provided!\n", __FILE__, __LINE__);
        return -1;
    }

    char options = 0;

    for (size_t i = 1; i < (size_t)argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (argv[i][1] == 'h' || strcmp(argv[i], "--help") == 0)
            {
                printf("%s", fullhelp);
                return 0;
            }
            if (argv[i][1] == 'v' || strcmp(argv[i], "--verbose") == 0)
            {
                options |= VERBOSE;
                continue;
            }
            if (argv[i][1] == 's' || strcmp(argv[i], "--strict") == 0)
            {
                options |= STRICT;
                continue;
            }
            if (argv[i][1] == 'c' || strcmp(argv[i], "--compile") == 0)
            {
                options |= COMPILE;
                continue;
            }
            if (argv[i][1] == 'x' || strcmp(argv[i], "--execute") == 0)
            {
                options |= RUN_COMPILED;
                continue;
            }
            if (argv[i][1] == 't' || strcmp(argv[i], "--transpile") == 0)
            {
                options |= TRANSPILE_C;
                continue;
            }

            printf("%s", usestr);
            return 0;
        }
    }

    struct program *program = program_init(argv[1]);
    if (program == NULL)
    {
        fprintf(stderr, "Error creating program!\n");
        return -1;
    }
    program->options = options;

    program_run(program);

    program_close(program);

    return 0;
}
