#pragma once

#include "caffeine_iterator.h"
#include "caffeine_ord_container_type.h"

typedef struct {
  uint64_t capacity;
  cff_size data_size;
  cff_err_e error_code;
  uintptr_t buffer;
  cff_order_function comparer;
  uint64_t count;
} cff_ord_vector_s;

cff_ord_vector_s cff_ord_vector_create(
    cff_size data_size, uint64_t capacity,
    cff_ord_e (*comparer_fn)(uintptr_t a, uintptr_t b, cff_size data_size),
    cff_allocator allocator);
cff_iterator_s cff_ord_vector_get_iterator(cff_ord_vector_s *vector);
cff_ord_vector_s cff_ord_vector_copy(cff_ord_vector_s vector,
                                     cff_allocator allocator);

cff_err_e cff_ord_vector_get(cff_ord_vector_s vector, uint64_t index,
                             uintptr_t out);
cff_err_e cff_ord_vector_get_ref(cff_ord_vector_s vector, uint64_t index,
                                 uintptr_t *out);
cff_err_e cff_ord_vector_add(cff_ord_vector_s *vector, uintptr_t in,
                             uint64_t *out_index, cff_allocator allocator);
cff_err_e cff_ord_vector_remove(cff_ord_vector_s *vector, uint64_t index,
                                cff_allocator allocator);
cff_err_e cff_ord_vector_destroy(cff_ord_vector_s vector,
                                 cff_allocator allocator);
cff_err_e cff_ord_vector_reserve(cff_ord_vector_s *vector, uint64_t capacity,
                                 cff_allocator allocator);

bool cff_ord_vector_is_valid(cff_ord_vector_s vector);
bool cff_ord_vector_equals(cff_ord_vector_s vector, cff_ord_vector_s other);
bool cff_ord_vector_contains(cff_ord_vector_s vector, uintptr_t value);

void cff_ord_vector_fill(cff_ord_vector_s vector, uintptr_t value,
                         cff_size value_size);
