#pragma once
#include <stdint.h>
#include "caffeine_memory.h"

#define cff_release(A) cff_mem_release((void *)A)

#define cff_arr_dcltype(NAME, TYPE) \
    typedef struct                  \
    {                               \
        TYPE *buffer;               \
        uint32_t count;             \
        uint32_t capacity;          \
    } NAME

#define cff_arr_init(ARR_PTR, CAPACITY)                                       \
    do                                                                        \
    {                                                                         \
        __typeof__(ARR_PTR) __m_ptr_1 = (ARR_PTR);                            \
        uint32_t c = (uint32_t)(CAPACITY);                                    \
        __m_ptr_1->count = 0;                                                 \
        __m_ptr_1->capacity = c;                                              \
        __m_ptr_1->buffer = cff_new_arr(__typeof__(__m_ptr_1->buffer[0]), c); \
    } while (0)

#define cff_arr_resize(ARR_PTR, CAPACITY)                                                      \
    do                                                                                         \
    {                                                                                          \
        __typeof__(ARR_PTR) __m_ptr_2 = (ARR_PTR);                                             \
        uint32_t __m_cap = (uint32_t)CAPACITY;                                                 \
        __typeof__(__m_ptr_2->buffer) __m_tmp_bf = cff_resize_arr(__m_ptr_2->buffer, __m_cap); \
        if (__m_tmp_bf != NULL)                                                                \
        {                                                                                      \
            __m_ptr_2->buffer = __m_tmp_bf;                                                    \
            __m_ptr_2->capacity = __m_cap;                                                     \
        }                                                                                      \
    } while (0);

#define cff_arr_add(ARR_PTR, VALUE)                             \
    do                                                          \
    {                                                           \
        __typeof__(ARR_PTR) __m_ptr_3 = (ARR_PTR);              \
        __typeof__(VALUE) __m_value = (VALUE);                  \
        if (__m_ptr_3->count == __m_ptr_3->capacity)            \
        {                                                       \
            cff_arr_resize(__m_ptr_3, __m_ptr_3->capacity * 2); \
        }                                                       \
        __m_ptr_3->buffer[__m_ptr_3->count] = __m_value;        \
        __m_ptr_3->count++;                                     \
    } while (0)

#define cff_arr_set(ARR_PTR, VALUE, INDEX) (ARR_PTR)->buffer[(uint32_t)(INDEX)] = (VALUE)

#define cff_arr_release(ARR_PTR) cff_release((ARR_PTR)->buffer);

#define cff_arr_get(ARR_PTR, INDEX) (ARR_PTR)->buffer[(uint32_t)(INDEX)];