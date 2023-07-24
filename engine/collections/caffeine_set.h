#pragma once

#include "../caffeine_types.h"
#include "../core/caffeine_memory.h"
#include "caffeine_iterator.h"

typedef struct {
  cff_size data_size;
  cff_comparer_function data_cmp_fn;
  cff_hash_function hash_fn;
  uint64_t capacity;
  uint64_t count;
  uint64_t collision_count;
  cff_err_e error_code;
  void *data;
  bool *used_slot;
} cff_set_s;

cff_set_s cff_set_create(cff_size data_size, uint64_t capacity,
                         cff_comparer_function comparer_fn,
                         cff_hash_function hash_fn, cff_allocator allocator);

bool cff_set_add(cff_set_s *set, uintptr_t value, cff_allocator allocator);
bool cff_set_remove(cff_set_s *set, uintptr_t value, cff_allocator allocator);
bool cff_set_get_index(cff_set_s set, uintptr_t value, uint64_t *index);
bool cff_set_get_contains(cff_set_s set, uintptr_t value);

void cff_set_clear(cff_set_s *set);
void cff_set_destroy(cff_set_s *set, cff_allocator allocator);
cff_set_s cff_set_copy(cff_set_s set, cff_allocator allocator);
cff_set_s cff_set_clone(cff_set_s set, cff_allocator allocator);
cff_iterator_s cff_set_get_iterator(cff_set_s *set);