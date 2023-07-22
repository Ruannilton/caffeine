#pragma once

#include "../caffeine_types.h"
#include "../core/caffeine_memory.h"
#include "caffeine_iterator.h"


typedef struct {
  cff_size data_size;
  cff_size key_size;
  uint64_t capacity;
  uint64_t count;
  uint64_t collision_count;
  cff_err_e error_code;
  cff_comparer_function key_cmp_fn;
  cff_comparer_function data_cmp_fn;
  cff_hash_function hash_fn;
  void *data;
  void *keys;
  bool *used_slot;
} cff_hashmap_s;

cff_hashmap_s cff_hashmap_create(cff_size key_size, cff_size data_size,
                                 uint64_t capacity,
                                 cff_comparer_function key_cmp_fn,
                                 cff_comparer_function data_cmp_fn,
                                 cff_hash_function hash_fn,
                                 cff_allocator_t allocator);
bool cff_hashmap_add(cff_hashmap_s hashmap, uintptr_t key, uintptr_t value,
                     cff_allocator_t allocator);
bool cff_hashmap_get(cff_hashmap_s hashmap, uintptr_t key, uintptr_t *value);
bool cff_hashmap_remove(cff_hashmap_s hashmap, uintptr_t key,
                        cff_allocator_t allocator);
bool cff_hashmap_contains_key(cff_hashmap_s hashmap, uintptr_t key);
bool cff_hashmap_contains_value(cff_hashmap_s hashmap, uintptr_t value);
void cff_hashmap_clear(cff_hashmap_s hashmap);
void cff_hashmap_destroy(cff_hashmap_s hashmap, cff_allocator_t allocator);
cff_hashmap_s cff_hashmap_copy(cff_hashmap_s hashmap,
                               cff_allocator_t allocator);
cff_hashmap_s cff_hashmap_clone(cff_hashmap_s hashmap,
                                cff_allocator_t allocator);
cff_iterator_s cff_hashmap_get_iterator(cff_hashmap_s *array);
