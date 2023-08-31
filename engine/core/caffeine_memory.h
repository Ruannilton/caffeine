#pragma once
#include "../caffeine_types.h"

struct cff_allocator_t;

typedef struct cff_allocator_t *cff_allocator;

#define CFF_GLOBAL_ALLOC (cff_memory_get_global())

cff_allocator cff_memory_get_global();

void cff_memory_init();
void cff_memory_end();

cff_err_e cff_memory_alloc(cff_size size, uintptr_t *out);
cff_err_e cff_memory_realloc(uintptr_t ptr, cff_size size, uintptr_t *out);
cff_err_e cff_memory_release(uintptr_t ptr);
cff_err_e cff_memory_get_size(uintptr_t ptr, cff_size *size);

cff_err_e cff_allocator_alloc(cff_allocator alloc, cff_size size,
                              uintptr_t *out);

cff_err_e cff_allocator_realloc(cff_allocator alloc, uintptr_t ptr,
                                cff_size size, uintptr_t *out);
cff_err_e cff_allocator_release(cff_allocator alloc, uintptr_t ptr);

cff_err_e cff_allocator_get_size(cff_allocator alloc, uintptr_t ptr,
                                 cff_size *size);

void cff_mem_copy(const void *from, void *dest, uint64_t size);
void cff_mem_move(const void *from, void *dest, uint64_t size);
bool cff_mem_cmp(const void *const from, const void *const dest, uint64_t size);
void cff_mem_set(const void *data, void *dest, uint64_t data_size,
                 uint64_t buffer_lenght);
void cff_mem_zero(void *dest, uint64_t data_size, uint64_t buffer_lenght);
