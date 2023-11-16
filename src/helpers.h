#ifndef AURORA_HELPERS_H
#define AURORA_HELPERS_H

#include <stdbool.h>
#include <stdint.h>

typedef uint32_t address;

inline int compare_bytes(char *x, char *y, size_t bytes)
{
    for (size_t i = 0; i < bytes; i++)
    {
        if (x[i] ^ y[i])
        {
            return 0;
        }
    }

    return 1;
}

inline void set_bytes(char *dst, char *src, size_t bytes)
{
    for (size_t i = 0; i < bytes; i++)
    {
        dst[i] = src[i];
    }
}

#endif