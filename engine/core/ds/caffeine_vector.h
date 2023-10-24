#pragma once

#include <stdint.h>
#include "../caffeine_memory.h"

#define cff_release(A) cff_mem_release((void *)A)

#define alloc_gen_array(PTR, CAPACITY) PTR = cff_new_arr(__typeof__(PTR[0]), CAPACITY)

#define cff_arr_dcltype(NAME, TYPE) \
    typedef struct                  \
    {                               \
        TYPE *buffer;               \
        uint32_t count;             \
        uint32_t capacity;          \
    } NAME

#define cff_arr_init(ARR_PTR, CAPACITY)            \
    do                                             \
    {                                              \
        __typeof__(ARR_PTR) __m_ptr_1 = (ARR_PTR); \
        uint32_t c = (uint32_t)(CAPACITY);         \
        __m_ptr_1->count = 0;                      \
        __m_ptr_1->capacity = c;                   \
        alloc_gen_array(__m_ptr_1->buffer, c);     \
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

#define cff_arr_add_at(ARR_PTR, VALUE, INDEX)                   \
    do                                                          \
    {                                                           \
        __typeof__(ARR_PTR) __m_ptr_3 = (ARR_PTR);              \
        __typeof__(VALUE) __m_value = (VALUE);                  \
        if (__m_ptr_3->count == __m_ptr_3->capacity)            \
        {                                                       \
            cff_arr_resize(__m_ptr_3, __m_ptr_3->capacity * 2); \
        }                                                       \
        __m_ptr_3->buffer[(uint32_t)(INDEX)] = __m_value;       \
        __m_ptr_3->count++;                                     \
    } while (0)

#define cff_arr_ordered_add(ARR_PTR, VALUE)                                                        \
    do                                                                                             \
    {                                                                                              \
        __typeof__(ARR_PTR) __m_ptr_3 = (ARR_PTR);                                                 \
        __typeof__(VALUE) __m_value = (VALUE);                                                     \
        if (__m_ptr_3->count == __m_ptr_3->capacity)                                               \
        {                                                                                          \
            cff_arr_resize(__m_ptr_3, __m_ptr_3->capacity * 2);                                    \
        }                                                                                          \
        uint32_t __m_i;                                                                            \
        for (__m_i = 0; __m_i < __m_ptr_3->count && __m_ptr_3->buffer[__m_i] < __m_value; __m_i++) \
            ;                                                                                      \
        if (__m_i < __m_ptr_3->count)                                                              \
        {                                                                                          \
            for (uint32_t __m_j = __m_ptr_3->count; __m_j > __m_i; __m_j--)                        \
            {                                                                                      \
                __m_ptr_3->buffer[__m_j] = __m_ptr_3->buffer[__m_j - 1];                           \
            }                                                                                      \
        }                                                                                          \
        __m_ptr_3->buffer[__m_i] = __m_value;                                                      \
        __m_ptr_3->count++;                                                                        \
    } while (0)

#define cff_arr_add_i(ARR_PTR, VALUE, OUT_INDEX)                \
    do                                                          \
    {                                                           \
        __typeof__(ARR_PTR) __m_ptr_3 = (ARR_PTR);              \
        __typeof__(VALUE) __m_value = (VALUE);                  \
        if (__m_ptr_3->count == __m_ptr_3->capacity)            \
        {                                                       \
            cff_arr_resize(__m_ptr_3, __m_ptr_3->capacity * 2); \
        }                                                       \
        OUT_INDEX = __m_ptr_3->count;                           \
        __m_ptr_3->buffer[__m_ptr_3->count] = __m_value;        \
        __m_ptr_3->count++;                                     \
    } while (0)

#define cff_arr_set(ARR_PTR, VALUE, INDEX) (ARR_PTR)->buffer[(uint32_t)(INDEX)] = (VALUE)

#define cff_arr_release(ARR_PTR) cff_release((ARR_PTR)->buffer)

#define cff_arr_get(ARR_PTR, INDEX) (ARR_PTR)->buffer[(uint32_t)(INDEX)]

#define cff_arr_get_ref(ARR_PTR, INDEX) (ARR_PTR)->buffer + ((uint32_t)(INDEX))

#define cff_arr_impl(ARRAY_NAME, TYPE)                                        \
    void ARRAY_NAME##_init(ARRAY_NAME *arr, uint32_t capacity)                \
    {                                                                         \
        arr->count = 0;                                                       \
        arr->capacity = capacity;                                             \
        alloc_gen_array(arr->buffer, capacity);                               \
    }                                                                         \
                                                                              \
    void ARRAY_NAME##_resize(ARRAY_NAME *arr, uint32_t capacity)              \
    {                                                                         \
        TYPE *tmp_buffer = cff_resize_arr(arr->buffer, capacity);             \
        if (tmp_buffer != NULL)                                               \
        {                                                                     \
            arr->buffer = tmp_buffer;                                         \
            arr->capacity = capacity;                                         \
        }                                                                     \
    }                                                                         \
                                                                              \
    void ARRAY_NAME##_add(ARRAY_NAME *arr, TYPE value)                        \
    {                                                                         \
        if (arr->count == arr->capacity)                                      \
        {                                                                     \
            ARRAY_NAME##_resize(arr, arr->capacity * 2);                      \
        }                                                                     \
        arr->buffer[arr->count] = value;                                      \
        arr->count++;                                                         \
    }                                                                         \
                                                                              \
    void ARRAY_NAME##_add_at(ARRAY_NAME *arr, TYPE value, uint32_t index)     \
    {                                                                         \
        if (arr->count == arr->capacity)                                      \
        {                                                                     \
            ARRAY_NAME##_resize(arr, arr->capacity * 2);                      \
        }                                                                     \
        arr->buffer[index] = value;                                           \
        arr->count++;                                                         \
    }                                                                         \
                                                                              \
    void ARRAY_NAME##_add_i(ARRAY_NAME *arr, TYPE value, uint32_t *out_index) \
    {                                                                         \
        if (arr->count == arr->capacity)                                      \
        {                                                                     \
            ARRAY_NAME##_resize(arr, arr->capacity * 2);                      \
        }                                                                     \
        *out_index = arr->count;                                              \
        arr->buffer[arr->count] = value;                                      \
        arr->count++;                                                         \
    }                                                                         \
                                                                              \
    void ARRAY_NAME##_set(ARRAY_NAME *arr, TYPE value, uint32_t index)        \
    {                                                                         \
        arr->buffer[index] = value;                                           \
    }                                                                         \
                                                                              \
    void ARRAY_NAME##_release(ARRAY_NAME *arr)                                \
    {                                                                         \
        cff_release(arr->buffer);                                             \
    }                                                                         \
                                                                              \
    TYPE ARRAY_NAME##_get(const ARRAY_NAME *arr, uint32_t index)              \
    {                                                                         \
        return arr->buffer[index];                                            \
    }                                                                         \
    TYPE *ARRAY_NAME##_get_ref(const ARRAY_NAME *arr, uint32_t index)         \
    {                                                                         \
        return arr->buffer + index;                                           \
    }                                                                         \
    int8_t ARRAY_NAME##_contains(const ARRAY_NAME *arr, TYPE value)           \
    {                                                                         \
        for (uint32_t i = 0; i < arr->count; i++)                             \
            if (cff_mem_cmp(arr->buffer + i, &value, sizeof(TYPE)))           \
                return 1;                                                     \
        return 0;                                                             \
    }                                                                         \
    void ARRAY_NAME##_zero(const ARRAY_NAME *arr) { cff_mem_zero(arr->buffer, sizeof(TYPE), sizeof(TYPE) * arr->capacity); }