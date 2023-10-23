#pragma once

#include <stdint.h>
#include "../caffeine_memory.h"
#include <string.h>
#include <stdio.h>

#define cff_release(A) cff_mem_release((void *)A)

#define alloc_gen_array(PTR, CAPACITY) PTR = cff_new_arr(__typeof__(PTR[0]), CAPACITY)

#define cff_hash_dcltype(NAME, KEY_TYPE, DATA_TYPE)                   \
    typedef struct                                                    \
    {                                                                 \
        DATA_TYPE *data_buffer;                                       \
        KEY_TYPE *key_buffer;                                         \
        uint8_t *used_slot;                                           \
        uint32_t (*hash_key_fn)(KEY_TYPE *, uint32_t);                \
        bool (*cmp_key_fn)(KEY_TYPE *, KEY_TYPE *);                   \
        bool (*cmp_data_fn)(DATA_TYPE *, DATA_TYPE *);                \
        uint32_t count;                                               \
        uint32_t capacity;                                            \
        uint32_t colision_count;                                      \
    } NAME;                                                           \
    uint32_t _generated_##NAME##_hash_fn(KEY_TYPE *data, uint32_t c); \
    bool _generated_##NAME##_cmp_key_fn(KEY_TYPE *a, KEY_TYPE *b);    \
    bool _generated_##NAME##_cmp_data_fn(DATA_TYPE *a, DATA_TYPE *b);

#define cff_hash_impl_default_functions(NAME, KEY_TYPE, DATA_TYPE)                                                \
    uint32_t _generated_##NAME##_hash_fn(KEY_TYPE *data, uint32_t c)                                              \
    {                                                                                                             \
        uint32_t hash = c;                                                                                        \
        const uint32_t size = (uint32_t)sizeof(KEY_TYPE);                                                         \
        const uint8_t *byte_data = (const uint8_t *)data;                                                         \
        for (size_t i = 0; i < size; i++)                                                                         \
        {                                                                                                         \
            hash = (hash * 31) + byte_data[i];                                                                    \
        }                                                                                                         \
                                                                                                                  \
        return hash;                                                                                              \
    }                                                                                                             \
    bool _generated_##NAME##_cmp_key_fn(KEY_TYPE *a, KEY_TYPE *b) { return memcmp(a, b, sizeof(KEY_TYPE)) == 0; } \
    bool _generated_##NAME##_cmp_data_fn(DATA_TYPE *a, DATA_TYPE *b) { return memcmp(a, b, sizeof(DATA_TYPE)) == 0; }

#define cff_hash_init(HASH_PTR, CAPACITY, HASH_KEY_FN, CMP_KEY_FN, CMP_DATA_FN)                \
    do                                                                                         \
    {                                                                                          \
        __typeof__(HASH_PTR) __m_ptr_3 = (HASH_PTR);                                           \
        uint32_t __m_capacity_3 = (uint32_t)(CAPACITY);                                        \
        __typeof__(HASH_KEY_FN) *__m_hash_key_fn_3 = (HASH_KEY_FN);                            \
        __typeof__(CMP_KEY_FN) *__m_cmp_key_fn_3 = (CMP_KEY_FN);                               \
        __typeof__(CMP_DATA_FN) *__m_cmp_data_fn_3 = (CMP_DATA_FN);                            \
        alloc_gen_array(__m_ptr_3->data_buffer, __m_capacity_3);                               \
        alloc_gen_array(__m_ptr_3->key_buffer, __m_capacity_3);                                \
        alloc_gen_array(__m_ptr_3->used_slot, __m_capacity_3);                                 \
        cff_mem_zero(__m_ptr_3->used_slot, sizeof(uint8_t), __m_capacity_3 * sizeof(uint8_t)); \
        __m_ptr_3->hash_key_fn = __m_hash_key_fn_3;                                            \
        __m_ptr_3->cmp_key_fn = __m_cmp_key_fn_3;                                              \
        __m_ptr_3->cmp_data_fn = __m_cmp_data_fn_3;                                            \
        __m_ptr_3->count = 0;                                                                  \
        __m_ptr_3->colision_count = 0;                                                         \
        __m_ptr_3->capacity = __m_capacity_3;                                                  \
    } while (0);

#define cff_hash_init_default(NAME, HASH_PTR, CAPACITY) cff_hash_init((HASH_PTR), (CAPACITY), (_generated_##NAME##_hash_fn), (_generated_##NAME##_cmp_key_fn), (_generated_##NAME##_cmp_data_fn))

#define cff_hash_add(HASH_PTR, KEY, DATA)                                                                         \
    do                                                                                                            \
    {                                                                                                             \
        __typeof__(HASH_PTR) __m_ptr_3 = (HASH_PTR);                                                              \
        __typeof__(KEY) __m_key_3 = (KEY);                                                                        \
        __typeof__(DATA) __m_data_3 = (DATA);                                                                     \
        if (__m_ptr_3->count > __m_ptr_3->capacity * 0.7)                                                         \
        {                                                                                                         \
            uint32_t __n_capacity = __m_ptr_3->capacity * 2;                                                      \
            DATA *__n_data_buffer;                                                                                \
            KEY *__n_key_buffer;                                                                                  \
            uint8_t *__n_used_buffer;                                                                             \
            uint32_t __n_max_collision = 0;                                                                       \
            alloc_gen_array(__n_data_buffer, __n_capacity);                                                       \
            alloc_gen_array(__n_key_buffer, __n_capacity);                                                        \
            alloc_gen_array(__n_used_buffer, __n_capacity);                                                       \
            cff_mem_zero(__n_used_buffer, sizeof(uint8_t), __n_capacity * sizeof(uint8_t));                       \
            for (uint32_t __m_i = 0; __m_i < __m_ptr_3->count; __m_i++)                                           \
            {                                                                                                     \
                __typeof__(__n_key_buffer[0]) __curr_key = __m_ptr_3->key_buffer[__m_i];                          \
                __typeof__(__n_data_buffer[0]) __curr_data = __m_ptr_3->data_buffer[__m_i];                       \
                uint32_t __hash_collision = 0;                                                                    \
                uint32_t __hash_index = (__m_ptr_3->hash_key_fn(&__curr_key, __hash_collision) % __n_capacity);   \
                while (__n_used_buffer[__hash_index])                                                             \
                {                                                                                                 \
                    __typeof__(KEY) __tmp_key = __n_key_buffer[__hash_index];                                     \
                    if (__m_ptr_3->cmp_key_fn(&__tmp_key, &__curr_key))                                           \
                    {                                                                                             \
                        __n_data_buffer[__hash_index] = __curr_data;                                              \
                        break;                                                                                    \
                    }                                                                                             \
                    __hash_collision++;                                                                           \
                    __hash_index = (__m_ptr_3->hash_key_fn(&__curr_key, __hash_collision) % __m_ptr_3->capacity); \
                }                                                                                                 \
                if (__hash_collision > __n_max_collision)                                                         \
                {                                                                                                 \
                    __n_max_collision = __hash_collision;                                                         \
                }                                                                                                 \
                __n_data_buffer[__hash_index] = __curr_data;                                                      \
                __n_key_buffer[__hash_index] = __curr_key;                                                        \
                __n_used_buffer[__hash_index] = 1;                                                                \
            }                                                                                                     \
            cff_release(__m_ptr_3->data_buffer);                                                                  \
            cff_release(__m_ptr_3->key_buffer);                                                                   \
            cff_release(__m_ptr_3->used_slot);                                                                    \
            __m_ptr_3->data_buffer = __n_data_buffer;                                                             \
            __m_ptr_3->key_buffer = __n_key_buffer;                                                               \
            __m_ptr_3->used_slot = __n_used_buffer;                                                               \
            __m_ptr_3->capacity = __n_capacity;                                                                   \
        }                                                                                                         \
        {                                                                                                         \
            uint32_t __hash_collision = 0;                                                                        \
            uint32_t __hash_index = (__m_ptr_3->hash_key_fn(&__m_key_3, __hash_collision) % __m_ptr_3->capacity); \
            while (__m_ptr_3->used_slot[__hash_index])                                                            \
            {                                                                                                     \
                __typeof__(KEY) __tmp_key = __m_ptr_3->key_buffer[__hash_index];                                  \
                if (__m_ptr_3->cmp_key_fn(&__m_key_3, &__tmp_key))                                                \
                {                                                                                                 \
                    __m_ptr_3->data_buffer[__hash_index] = __m_data_3;                                            \
                    break;                                                                                        \
                }                                                                                                 \
                __hash_collision++;                                                                               \
                __hash_index = (__m_ptr_3->hash_key_fn(&__m_key_3, __hash_collision) % __m_ptr_3->capacity);      \
            }                                                                                                     \
            if (__hash_collision > __m_ptr_3->colision_count)                                                     \
            {                                                                                                     \
                __m_ptr_3->colision_count = __hash_collision;                                                     \
            }                                                                                                     \
            __m_ptr_3->data_buffer[__hash_index] = __m_data_3;                                                    \
            __m_ptr_3->key_buffer[__hash_index] = __m_key_3;                                                      \
            __m_ptr_3->used_slot[__hash_index] = 1;                                                               \
            __m_ptr_3->count++;                                                                                   \
        }                                                                                                         \
    } while (0);

#define cff_hash_get(HASH_PTR, KEY, RESULT) ({                                                            \
    __typeof__(HASH_PTR) __m_ptr_3 = (HASH_PTR);                                                          \
    __typeof__(KEY) __m_key_3 = (KEY);                                                                    \
    uint32_t __hash_collision = 0;                                                                        \
    uint32_t __hash_index = (__m_ptr_3->hash_key_fn(&__m_key_3, __hash_collision) % __m_ptr_3->capacity); \
    int8_t __m_found = 0;                                                                                 \
    while (__m_ptr_3->used_slot[__hash_index])                                                            \
    {                                                                                                     \
        __typeof__(KEY) __tmp_key = __m_ptr_3->key_buffer[__hash_index];                                  \
        if (__m_ptr_3->cmp_key_fn(&__m_key_3, &__tmp_key))                                                \
        {                                                                                                 \
            RESULT = __m_ptr_3->data_buffer[__hash_index];                                                \
            __m_found = 1;                                                                                \
            break;                                                                                        \
        }                                                                                                 \
        __hash_collision++;                                                                               \
        if (__hash_collision > __m_ptr_3->colision_count)                                                 \
            break;                                                                                        \
        __hash_index = (__m_ptr_3->hash_key_fn(&__m_key_3, __hash_collision) % __m_ptr_3->capacity);      \
    };                                                                                                    \
    __m_found;                                                                                            \
})

#define cff_hash_exist(HASH_PTR, KEY) ({                                                                   \
    __typeof__(HASH_PTR) __m_ptr_3 = (HASH_PTR);                                                           \
    __typeof__(KEY) __m_key_3 = (KEY);                                                                     \
    uint32_t __hash_collision = 0;                                                                         \
    uint32_t __hash_index = (__m_ptr_3->hash_key_fn(&__m_key_3, &__hash_collision) % __m_ptr_3->capacity); \
    int8_t __m_found = 0;                                                                                  \
    while (__m_ptr_3->used_slot[__hash_index])                                                             \
    {                                                                                                      \
        __typeof__(KEY) __tmp_key = __m_ptr_3->key_buffer[__hash_index];                                   \
        if (__m_ptr_3->cmp_key_fn(&__m_key_3, &__tmp_key))                                                 \
        {                                                                                                  \
            __m_found = 1;                                                                                 \
            break;                                                                                         \
        }                                                                                                  \
        __hash_collision++;                                                                                \
        if (__hash_collision > __m_ptr_3->colision_count)                                                  \
            break;                                                                                         \
        __hash_index = (__m_ptr_3->hash_key_fn(&__m_key_3, __hash_collision) % __m_ptr_3->capacity);       \
    };                                                                                                     \
    __m_found;                                                                                             \
})

#define cff_hash_remove(HASH_PTR, KEY) ({                                                                 \
    __typeof__(HASH_PTR) __m_ptr_3 = (HASH_PTR);                                                          \
    __typeof__(KEY) __m_key_3 = (KEY);                                                                    \
    uint32_t __hash_collision = 0;                                                                        \
    uint32_t __hash_index = (__m_ptr_3->hash_key_fn(&__m_key_3, __hash_collision) % __m_ptr_3->capacity); \
    int8_t __m_found = 0;                                                                                 \
    while (__m_ptr_3->used_slot[__hash_index])                                                            \
    {                                                                                                     \
        __typeof__(KEY) __tmp_key = __m_ptr_3->key_buffer[__hash_index];                                  \
        if (__m_ptr_3->cmp_key_fn(&__m_key_3, &__tmp_key))                                                \
        {                                                                                                 \
            __m_ptr_3->used_slot[__hash_index] = 0;                                                       \
            __m_ptr_3->count--;                                                                           \
            __m_found = 1;                                                                                \
            break;                                                                                        \
        }                                                                                                 \
        __hash_collision++;                                                                               \
        if (__hash_collision > __m_ptr_3->colision_count)                                                 \
            break;                                                                                        \
        __hash_index = (__m_ptr_3->hash_key_fn(&__m_key_3, __hash_collision) % __m_ptr_3->capacity);      \
    };                                                                                                    \
    __m_found;                                                                                            \
})

#define cff_hash_release(HASH_PTR)               \
    do                                           \
    {                                            \
        __typeof__(HASH_PTR) m_ptr = (HASH_PTR); \
        cff_release(m_ptr->data_buffer);         \
        cff_release(m_ptr->key_buffer);          \
        cff_release(m_ptr->used_slot);           \
    } while (0);

#define cff_hash_impl(NAME, KEY_TYPE, DATA_TYPE)                                                      \
    cff_hash_impl_default_functions(NAME, KEY_TYPE, DATA_TYPE);                                       \
    void NAME##_init(NAME *hash_ptr, uint32_t capacity,                                               \
                     uint32_t (*hash_key_fn)(KEY_TYPE *, uint32_t),                                   \
                     bool (*cmp_key_fn)(KEY_TYPE *, KEY_TYPE *),                                      \
                     bool (*cmp_data_fn)(DATA_TYPE *, DATA_TYPE *))                                   \
    {                                                                                                 \
        NAME *m_ptr = hash_ptr;                                                                       \
        m_ptr->hash_key_fn = hash_key_fn;                                                             \
        m_ptr->cmp_key_fn = cmp_key_fn;                                                               \
        m_ptr->cmp_data_fn = cmp_data_fn;                                                             \
        m_ptr->count = 0;                                                                             \
        m_ptr->colision_count = 0;                                                                    \
        m_ptr->capacity = capacity;                                                                   \
        alloc_gen_array(m_ptr->data_buffer, capacity);                                                \
        alloc_gen_array(m_ptr->key_buffer, capacity);                                                 \
        alloc_gen_array(m_ptr->used_slot, capacity);                                                  \
        cff_mem_zero(m_ptr->used_slot, sizeof(uint8_t), capacity * sizeof(uint8_t));                  \
    }                                                                                                 \
    uint32_t NAME##_resolve_collision(NAME *m_ptr, KEY_TYPE key, uint32_t hash_index, DATA_TYPE data) \
    {                                                                                                 \
        uint32_t hash_collision = 0;                                                                  \
        while (m_ptr->used_slot[hash_index])                                                          \
        {                                                                                             \
            KEY_TYPE tmp_key = m_ptr->key_buffer[hash_index];                                         \
            if (m_ptr->cmp_key_fn(&key, &tmp_key))                                                    \
            {                                                                                         \
                m_ptr->data_buffer[hash_index] = data;                                                \
                return hash_collision;                                                                \
            }                                                                                         \
            hash_collision++;                                                                         \
            hash_index = (m_ptr->hash_key_fn(&key, hash_collision) % m_ptr->capacity);                \
        }                                                                                             \
        m_ptr->data_buffer[hash_index] = data;                                                        \
        m_ptr->key_buffer[hash_index] = key;                                                          \
        m_ptr->used_slot[hash_index] = 1;                                                             \
        return hash_collision;                                                                        \
    }                                                                                                 \
                                                                                                      \
    void NAME##_resize(NAME *m_ptr, uint32_t new_capacity)                                            \
    {                                                                                                 \
        printf("rez\n");                                                                              \
        DATA_TYPE *n_data_buffer = cff_new_arr(DATA_TYPE, new_capacity);                              \
        KEY_TYPE *n_key_buffer = cff_new_arr(KEY_TYPE, new_capacity);                                 \
        uint8_t *n_used_buffer = cff_new_arr(uint8_t, new_capacity);                                  \
        uint32_t n_max_collision = 0;                                                                 \
        cff_mem_zero(n_used_buffer, sizeof(uint8_t), new_capacity * sizeof(uint8_t));                 \
        for (uint32_t m_i = 0; m_i < m_ptr->count; m_i++)                                             \
        {                                                                                             \
            KEY_TYPE curr_key = m_ptr->key_buffer[m_i];                                               \
            DATA_TYPE curr_data = m_ptr->data_buffer[m_i];                                            \
            uint32_t hash_collision = 0;                                                              \
            uint32_t hash_index = m_ptr->hash_key_fn(&curr_key, hash_collision) % new_capacity;       \
            hash_collision += NAME##_resolve_collision(m_ptr, curr_key, hash_index, curr_data);       \
            if (hash_collision > n_max_collision)                                                     \
            {                                                                                         \
                n_max_collision = hash_collision;                                                     \
            }                                                                                         \
        }                                                                                             \
        cff_release(m_ptr->data_buffer);                                                              \
        cff_release(m_ptr->key_buffer);                                                               \
        cff_release(m_ptr->used_slot);                                                                \
        m_ptr->data_buffer = n_data_buffer;                                                           \
        m_ptr->key_buffer = n_key_buffer;                                                             \
        m_ptr->used_slot = n_used_buffer;                                                             \
        m_ptr->capacity = new_capacity;                                                               \
    }                                                                                                 \
                                                                                                      \
    void NAME##_add(NAME *hash_ptr, KEY_TYPE key, DATA_TYPE data)                                     \
    {                                                                                                 \
        if (hash_ptr->count > hash_ptr->capacity * 0.7)                                               \
        {                                                                                             \
            printf("sem vaga\n");                                                                     \
            uint32_t new_capacity = hash_ptr->capacity * 2;                                           \
            NAME##_resize(hash_ptr, new_capacity);                                                    \
        }                                                                                             \
        uint32_t hash_collision = 0;                                                                  \
        uint32_t hash_index = (hash_ptr->hash_key_fn(&key, hash_collision) % hash_ptr->capacity);     \
        hash_collision += NAME##_resolve_collision(hash_ptr, key, hash_index, data);                  \
        if (hash_collision > hash_ptr->colision_count)                                                \
        {                                                                                             \
            hash_ptr->colision_count = hash_collision;                                                \
        }                                                                                             \
        hash_ptr->count++;                                                                            \
    }                                                                                                 \
    int8_t NAME##_get(NAME *m_ptr, KEY_TYPE key, DATA_TYPE *result)                                   \
    {                                                                                                 \
        uint32_t hash_collision = 0;                                                                  \
        uint32_t hash_index = (m_ptr->hash_key_fn(&key, hash_collision) % m_ptr->capacity);           \
        int8_t found = 0;                                                                             \
        while (m_ptr->used_slot[hash_index])                                                          \
        {                                                                                             \
            KEY_TYPE tmp_key = m_ptr->key_buffer[hash_index];                                         \
            if (m_ptr->cmp_key_fn(&key, &tmp_key))                                                    \
            {                                                                                         \
                *result = m_ptr->data_buffer[hash_index];                                             \
                found = 1;                                                                            \
                break;                                                                                \
            }                                                                                         \
            hash_collision++;                                                                         \
            if (hash_collision > m_ptr->colision_count)                                               \
                break;                                                                                \
            hash_index = (m_ptr->hash_key_fn(&key, hash_collision) % m_ptr->capacity);                \
        }                                                                                             \
        return found;                                                                                 \
    }                                                                                                 \
    int8_t NAME##_exist(NAME *m_ptr, KEY_TYPE key)                                                    \
    {                                                                                                 \
        uint32_t hash_collision = 0;                                                                  \
        uint32_t hash_index = (m_ptr->hash_key_fn(&key, hash_collision) % m_ptr->capacity);           \
        int8_t found = 0;                                                                             \
        while (m_ptr->used_slot[hash_index])                                                          \
        {                                                                                             \
            KEY_TYPE tmp_key = m_ptr->key_buffer[hash_index];                                         \
            if (m_ptr->cmp_key_fn(&key, &tmp_key))                                                    \
            {                                                                                         \
                found = 1;                                                                            \
                break;                                                                                \
            }                                                                                         \
            hash_collision++;                                                                         \
            if (hash_collision > m_ptr->colision_count)                                               \
                break;                                                                                \
            hash_index = (m_ptr->hash_key_fn(&key, hash_collision) % m_ptr->capacity);                \
        }                                                                                             \
        return found;                                                                                 \
    }                                                                                                 \
    int8_t NAME##_remove(NAME *m_ptr, KEY_TYPE key)                                                   \
    {                                                                                                 \
        uint32_t hash_collision = 0;                                                                  \
        uint32_t hash_index = (m_ptr->hash_key_fn(&key, hash_collision) % m_ptr->capacity);           \
        int8_t found = 0;                                                                             \
        while (m_ptr->used_slot[hash_index])                                                          \
        {                                                                                             \
            KEY_TYPE tmp_key = m_ptr->key_buffer[hash_index];                                         \
            if (m_ptr->cmp_key_fn(&key, &tmp_key))                                                    \
            {                                                                                         \
                m_ptr->used_slot[hash_index] = 0;                                                     \
                m_ptr->count--;                                                                       \
                found = 1;                                                                            \
                break;                                                                                \
            }                                                                                         \
            hash_collision++;                                                                         \
            if (hash_collision > m_ptr->colision_count)                                               \
                break;                                                                                \
            hash_index = (m_ptr->hash_key_fn(&key, hash_collision) % m_ptr->capacity);                \
        }                                                                                             \
        return found;                                                                                 \
    }                                                                                                 \
    void NAME##_release(NAME *m_ptr)                                                                  \
    {                                                                                                 \
        cff_release(m_ptr->data_buffer);                                                              \
        cff_release(m_ptr->key_buffer);                                                               \
        cff_release(m_ptr->used_slot);                                                                \
    }
