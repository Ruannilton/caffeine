#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "caffeine_core.h"

typedef struct
{
    cff_size data_size;
    uint64_t capacity;
    uint64_t count;

    cff_comparer_function data_cmp_fn;
    cff_hash_function hash_fn;
    void *data;
} caffeine_set_s;
