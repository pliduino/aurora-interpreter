#ifndef AURORA_TYPED_ARRAY_H
#define AURORA_TYPED_ARRAY_H

enum typed_array_type
{
    INT,
    FLOAT,
};

struct typed_array
{
    void *pointer;
    enum array_type type;
    size_t size;
};

struct typed_array *typed_array_init(enum typed_array_type type, int size);

#endif