#pragma once
#include "../caffeine_types.h"

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
