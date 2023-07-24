#pragma once

#include "caffeine_container_type.h"
#include "caffeine_iterator.h"

typedef struct cff_linked_list_bucket_s cff_linked_list_bucket;

struct cff_linked_list_bucket_s {
  uintptr_t data;
  cff_linked_list_bucket *next;
};

typedef struct {
  uint64_t count;
  cff_size data_size;
  cff_err_e error_code;
  uintptr_t buffer;
  cff_linked_list_bucket *last;
} cff_linked_list_s;

cff_linked_list_s cff_linked_list_create(cff_size data_size);
cff_iterator_s cff_linked_list_get_iterator(cff_linked_list_s *linked_list);
cff_linked_list_s cff_linked_list_copy(cff_linked_list_s linked_list,
                                       cff_allocator allocator);

cff_err_e cff_linked_list_get(cff_linked_list_s linked_list, uint64_t index,
                              uintptr_t out);
cff_err_e cff_linked_list_get_ref(cff_linked_list_s linked_list, uint64_t index,
                                  uintptr_t *out);
cff_err_e cff_linked_list_set(cff_linked_list_s linked_list, uint64_t index,
                              uintptr_t in);
cff_err_e cff_linked_list_insert(cff_linked_list_s *linked_list, uint64_t index,
                                 uintptr_t in, cff_allocator allocator);
cff_err_e cff_linked_list_remove(cff_linked_list_s *linked_list, uint64_t index,
                                 cff_allocator allocator);
cff_err_e cff_linked_list_destroy(cff_linked_list_s linked_list,
                                  cff_allocator allocator);
cff_err_e cff_linked_list_reserve(cff_linked_list_s *linked_list,
                                  uint64_t capacity, cff_allocator allocator);
cff_err_e cff_linked_list_push_back(cff_linked_list_s *linked_list,
                                    uintptr_t in, cff_allocator allocator);
cff_err_e cff_linked_list_pop_back(cff_linked_list_s *linked_list,
                                   cff_allocator allocator);
cff_err_e cff_linked_list_push_front(cff_linked_list_s *linked_list,
                                     uintptr_t in, cff_allocator allocator);
cff_err_e cff_linked_list_pop_front(cff_linked_list_s *linked_list,
                                    cff_allocator allocator);

bool cff_linked_list_is_valid(cff_linked_list_s linked_list);
bool cff_linked_list_equals(cff_linked_list_s linked_list,
                            cff_linked_list_s other);
bool cff_linked_list_contains(cff_linked_list_s linked_list, uintptr_t value);

void cff_linked_list_fill(cff_linked_list_s linked_list, uintptr_t value,
                          cff_size value_size);
cff_err_e cff_linked_list_sort(cff_linked_list_s linked_list, uint64_t start,
                               uint64_t end, cff_order_function order);