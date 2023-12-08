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

void *cff_mem_realloc(const void *ptr_owning, uint64_t size);

void cff_mem_release(const void *const ptr_owning);

void cff_mem_copy(const void *const from_ref, void *const dest_mut_ref, uint64_t size);

void cff_mem_move(const void *const from_ref, void *const dest_mut_ref, uint64_t size);

bool cff_mem_cmp(const void *const from_ref, const void *const dest_mut_ref, uint64_t size);

void cff_mem_set(const void *const data_ref, void *const dest_mut_ref, uint64_t data_size,
                 uint64_t buffer_lenght);
void cff_mem_zero(void *const dest_mut_ref, uint64_t buffer_lenght);
