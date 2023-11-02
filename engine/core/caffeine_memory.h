#pragma once
#include "../caffeine_types.h"

#define cff_new(T) ((T *)cff_mem_alloc(sizeof(T)))
#define cff_new_arr(T, L) ((T *)cff_mem_alloc(sizeof(T) * (L)))
#define cff_resize_arr(A, L) ((__typeof__(*A) *)cff_mem_realloc((void *)(A), sizeof(__typeof__(*A)) * (L)))
#define cff_arr_copy(FROM, TO, LEN)                                               \
    TO = ((__typeof__(*FROM) *)cff_mem_alloc(sizeof(__typeof__(*FROM)) * (LEN))); \
    cff_mem_copy(FROM, TO, sizeof(__typeof__(*FROM)) * (LEN));

void cff_memory_init();
void cff_memory_end();

void *cff_mem_alloc(uint64_t size);

void *cff_mem_realloc(void *ptr, uint64_t size);

void cff_mem_release(void *ptr);

void cff_mem_copy(const void *from, void *dest, uint64_t size);

void cff_mem_move(const void *from, void *dest, uint64_t size);

bool cff_mem_cmp(const void *const from, const void *const dest, uint64_t size);

void cff_mem_set(const void *data, void *dest, uint64_t data_size,
                 uint64_t buffer_lenght);
void cff_mem_zero(void *dest, uint64_t data_size, uint64_t buffer_lenght);
