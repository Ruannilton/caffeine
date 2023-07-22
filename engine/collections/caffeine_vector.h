#pragma once

#include "caffeine_container_type.h"
#include "caffeine_iterator.h"

typedef struct {
  uint64_t capacity;
  cff_size data_size;
  cff_err_e error_code;
  uintptr_t buffer;
  uint64_t count;
} cff_vector_s;

cff_vector_s cff_vector_create(cff_size data_size, uint64_t capacity,
                               cff_allocator_t allocator);
cff_iterator_s cff_vector_get_iterator(cff_vector_s *vector);
cff_vector_s cff_vector_copy(cff_vector_s vector, cff_allocator_t allocator);

cff_err_e cff_vector_get(cff_vector_s vector, uint64_t index, uintptr_t out);
cff_err_e cff_vector_get_ref(cff_vector_s vector, uint64_t index,
                             uintptr_t *out);
cff_err_e cff_vector_set(cff_vector_s vector, uint64_t index, uintptr_t in);
cff_err_e cff_vector_insert(cff_vector_s *vector, uint64_t index, uintptr_t in,
                            cff_allocator_t allocator);
cff_err_e cff_vector_remove(cff_vector_s *vector, uint64_t index,
                            cff_allocator_t allocator);
cff_err_e cff_vector_destroy(cff_vector_s vector, cff_allocator_t allocator);
cff_err_e cff_vector_reserve(cff_vector_s *vector, uint64_t capacity,
                             cff_allocator_t allocator);
cff_err_e cff_vector_push_back(cff_vector_s *vector, uintptr_t in,
                               cff_allocator_t allocator);
cff_err_e cff_vector_pop_back(cff_vector_s *vector, cff_allocator_t allocator);
cff_err_e cff_vector_push_front(cff_vector_s *vector, uintptr_t in,
                                cff_allocator_t allocator);
cff_err_e cff_vector_pop_front(cff_vector_s *vector, cff_allocator_t allocator);

bool cff_vector_is_valid(cff_vector_s vector);
bool cff_vector_equals(cff_vector_s vector, cff_vector_s other);
bool cff_vector_contains(cff_vector_s vector, uintptr_t value);

void cff_vector_fill(cff_vector_s vector, uintptr_t value, cff_size value_size);
cff_err_e cff_vector_sort(cff_vector_s vector, uint64_t start, uint64_t end,
                          cff_order_function order);