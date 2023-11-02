#pragma once

#include <stdint.h>
#include "../caffeine_memory.h"

#define cff_release(A) cff_mem_release((void *)A)

#define alloc_gen_array(PTR, CAPACITY) PTR = cff_new_arr(__typeof__(PTR[0]), CAPACITY)

#define cff_sparse_dcltype(NAME, TYPE) \
    typedef struct                     \
    {                                  \
        TYPE *dense;                   \
        uint32_t *sparse;              \
        uint32_t count;                \
        uint32_t capacity;             \
    } NAME

#define cff_sparse_init(PTR, CAPACITY)                    \
    do                                                    \
    {                                                     \
        __typeof__(PTR) __m_ptr = (PTR);                  \
        uint32_t c = (uint32_t)(CAPACITY ? CAPACITY : 4); \
        __m_ptr->count = 0;                               \
        __m_ptr->capacity = c;                            \
        alloc_gen_array(__m_ptr->dense, c);               \
        alloc_gen_array(__m_ptr->sparse, c);              \
    } while (0);

#define cff_sparse_resize(PTR, CAPACITY)                                                           \
    do                                                                                             \
    {                                                                                              \
        __typeof__(PTR) __m_ptr = (PTR);                                                           \
        uint32_t __m_cap = (uint32_t)CAPACITY;                                                     \
        __typeof__(__m_ptr->dense) __m_tmp_dense = cff_resize_arr(__m_ptr->dense, __m_cap);        \
        if (__m_tmp_dense != NULL)                                                                 \
        {                                                                                          \
            __typeof__(__m_ptr->sparse) __m_tmp_sparse = cff_resize_arr(__m_ptr->sparse, __m_cap); \
            if (__m_tmp_sparse != NULL)                                                            \
            {                                                                                      \
                __m_ptr->dense = __m_tmp_dense;                                                    \
                __m_ptr->sparse = __m_tmp_sparse;                                                  \
                __m_ptr->capacity = __m_cap;                                                       \
            }                                                                                      \
        }                                                                                          \
    } while (0);

#define cff_sparse_add(PTR, DATA, OUT_INDEX)