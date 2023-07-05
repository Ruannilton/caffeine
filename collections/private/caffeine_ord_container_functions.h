#pragma once

#include "../caffeine_iterator.h"
#include "../caffeine_ord_container_type.h"
#include <caffeine_memory.h>

cff_ord_container_s cff_ord_container_create(
    cff_size data_size, uint64_t capacity,
    cff_ord_e (*comparer_fn)(uintptr_t a, uintptr_t b, cff_size data_size),
    cff_allocator_t allocator);
cff_iterator_s cff_ord_container_get_iterator(cff_ord_container_s *container);
cff_ord_container_s cff_ord_container_copy(cff_ord_container_s container,
                                           cff_allocator_t allocator);

cff_err_e cff_ord_container_get(cff_ord_container_s container, uint64_t index,
                                uintptr_t out);
cff_err_e cff_ord_container_get_ref(cff_ord_container_s container,
                                    uint64_t index, uintptr_t *out);
cff_err_e cff_ord_container_add(cff_ord_container_s container, uintptr_t in,
                                uint64_t *out_index);
cff_err_e cff_ord_container_remove(cff_ord_container_s container,
                                   uint64_t index);
cff_err_e cff_ord_container_destroy(cff_ord_container_s container,
                                    cff_allocator_t allocator);
cff_err_e cff_ord_container_resize(cff_ord_container_s *container,
                                   uint64_t new_capacity,
                                   cff_allocator_t allocator);
bool cff_ord_container_find_index(cff_ord_container_s container, uintptr_t in,
                                  uint64_t *out_index);

bool cff_ord_container_is_valid(cff_ord_container_s container);
bool cff_ord_container_equals(cff_ord_container_s container,
                              cff_ord_container_s other);
bool cff_ord_container_contains(cff_ord_container_s container, uintptr_t value);

void cff_ord_container_fill(cff_ord_container_s container, uintptr_t value,
                            cff_size value_size);
