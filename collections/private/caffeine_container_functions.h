#pragma once

#include "../caffeine_container_type.h"
#include "../caffeine_iterator.h"
#include <caffeine_memory.h>

cff_container_s cff_container_create(cff_size data_size, uint64_t capacity,
                                     cff_allocator_t allocator);
cff_iterator_s cff_container_get_iterator(cff_container_s *container);
cff_container_s cff_container_copy(cff_container_s container,
                                   cff_allocator_t allocator);

cff_err_e cff_container_get(cff_container_s container, uint64_t index,
                            uintptr_t out);
cff_err_e cff_container_get_ref(cff_container_s container, uint64_t index,
                                uintptr_t *out);
cff_err_e cff_container_set(cff_container_s container, uint64_t index,
                            uintptr_t in);
cff_err_e cff_container_insert(cff_container_s container, uint64_t index,
                               uintptr_t in);
cff_err_e cff_container_remove(cff_container_s container, uint64_t index);
cff_err_e cff_container_destroy(cff_container_s container,
                                cff_allocator_t allocator);
cff_err_e cff_container_resize(cff_container_s *container,
                               uint64_t new_capacity,
                               cff_allocator_t allocator);

bool cff_container_is_valid(cff_container_s container);
bool cff_container_equals(cff_container_s container, cff_container_s other);
bool cff_container_contains(cff_container_s container, uintptr_t value);

void cff_container_fill(cff_container_s container, uintptr_t value,
                        cff_size value_size);
cff_err_e cff_container_sort(cff_container_s container, uint64_t start,
                             uint64_t end, cff_order_function order);
