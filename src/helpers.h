#ifndef AURORA_HELPERS_H
#define AURORA_HELPERS_H

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

#endif