#pragma once

#include "../core/caffeine_types.h"
#include "caffeine_container_type.h"
#include "caffeine_iterator.h"

typedef struct {
  uint64_t capacity;
  cff_size data_size;
  cff_err_e error_code;
  uintptr_t buffer;
} cff_array_s;

cff_array_s cff_array_create(cff_size data_size, uint64_t capacity,
                             cff_allocator_t allocator);
cff_iterator_s cff_array_get_iterator(cff_array_s *array);
cff_array_s cff_array_copy(cff_array_s array, cff_allocator_t allocator);

cff_err_e cff_array_get(cff_array_s array, uint64_t index, uintptr_t out);
cff_err_e cff_array_get_ref(cff_array_s array, uint64_t index, uintptr_t *out);
cff_err_e cff_array_set(cff_array_s array, uint64_t index, uintptr_t in);
cff_err_e cff_array_insert(cff_array_s *array, uint64_t index, uintptr_t in,
                           cff_allocator_t allocator);
cff_err_e cff_array_remove(cff_array_s *array, uint64_t index,
                           cff_allocator_t allocator);
cff_err_e cff_array_destroy(cff_array_s array, cff_allocator_t allocator);

bool cff_array_is_valid(cff_array_s array);
bool cff_array_equals(cff_array_s array, cff_array_s other);
bool cff_array_contains(cff_array_s array, uintptr_t value);

void cff_array_fill(cff_array_s array, uintptr_t value, cff_size value_size);
cff_err_e cff_array_sort(cff_array_s array, uint64_t start, uint64_t end,
                         cff_order_function order);