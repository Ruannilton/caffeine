#pragma once

#include "private/caffeine_ord_container_functions.h"

static uint64_t cff_ord_container_get_capacity(cff_container_s *container) {
  return container->capacity;
}

static cff_size cff_ord_container_get_data_size(cff_container_s *container) {
  return container->data_size;
}

static bool cff_ord_container_get_item_ref(cff_container_s *container,
                                           uint64_t index, uintptr_t *out) {
  return cff_ord_container_get_ref(*(cff_ord_container_s *)container, index,
                                   out) == CFF_ERR_NONE;
}

static bool cff_ordered_container_find(cff_ord_container_s *container,
                                       uintptr_t ptr_in, uint64_t *out_index,
                                       cff_order_function comparer_function,
                                       uint64_t lenght) {

  if (cff_binary_search(container->buffer, ptr_in, 0, lenght,
                        container->data_size, comparer_function, out_index)) {
    return 1;
  }
  return 0;
}

static inline uint64_t
cff_ordered_container_get_index(cff_ord_container_s *container,
                                uintptr_t value_ptr, uint64_t count) {
  if (count == 0) {
    return 0;
  }
  if (count == 1) {
    uintptr_t value = 0;
    cff_ord_container_get_ref(*container, 0, &value);

    if (container->comparer(value_ptr, value, container->data_size) ==
        CFF_GREATER) {
      return 1;
    }
    return 0;
  }

  uint64_t idx = 0;
  if (cff_ordered_container_find(container, value_ptr, &idx,
                                 container->comparer, count)) {
    return idx++;
  }

  return idx;
}

cff_ord_container_s cff_ord_container_create(
    cff_size data_size, uint64_t capacity,
    cff_ord_e (*comparer_fn)(uintptr_t a, uintptr_t b, cff_size data_size),
    cff_allocator_t allocator) {

  uintptr_t buffer_address = (uintptr_t)(
      cff_allocator_allocate(&allocator, (cff_size)(capacity * data_size)));

  if (buffer_address == 0) {
    return (cff_ord_container_s){.error_code = CFF_ERR_ALLOC};
  }

  return (cff_ord_container_s){.data_size = data_size,
                               .capacity = capacity,
                               .error_code = CFF_ERR_NONE,
                               .buffer = buffer_address,
                               .comparer = comparer_fn};
}

cff_err_e cff_ord_container_get(cff_ord_container_s container, uint64_t index,
                                uintptr_t out) {
  debug_assert(out != 0);

  if (index >= container.capacity)
    return CFF_ERR_OUT_OF_BOUNDS;

  const void *from =
      (const void *)(container.buffer + (index * container.data_size));
  void *to = (void *)out;

  cff_mem_copy(from, to, container.data_size);

  return CFF_ERR_NONE;
}

cff_err_e cff_ord_container_get_ref(cff_ord_container_s container,
                                    uint64_t index, uintptr_t *out) {
  debug_assert((uintptr_t)out != 0);

  if (index >= container.capacity)
    return CFF_ERR_OUT_OF_BOUNDS;

  uintptr_t ref = (uintptr_t)(container.buffer + (index * container.data_size));

  *out = ref;

  return CFF_ERR_NONE;
}

cff_err_e cff_ord_container_add(cff_ord_container_s container, uintptr_t in,
                                uint64_t *out_index) {
  debug_assert(in != 0);

  uint64_t index =
      cff_ordered_container_get_index(&container, in, container.capacity);

  if (index >= container.capacity)
    return CFF_ERR_OUT_OF_BOUNDS;

  void *to = (void *)(container.buffer + (index * container.data_size));
  const void *from = (const void *)in;

  cff_mem_copy(from, to, container.data_size);

  if (out_index != NULL)
    *out_index = index;
  return CFF_ERR_NONE;
}

cff_err_e cff_ord_container_remove(cff_ord_container_s container,
                                   uint64_t index) {
  if (index >= container.capacity)
    return CFF_ERR_OUT_OF_BOUNDS;

  uint64_t payload_size =
      ((container.capacity - 1) - index) * container.data_size;

  const void *from =
      (const void *)(container.buffer + (index + 1) * container.data_size);
  void *to = (void *)(container.buffer + index * container.data_size);

  cff_mem_move(from, to, (cff_size)payload_size);

  return CFF_ERR_NONE;
}

cff_err_e cff_ord_container_destroy(cff_ord_container_s container,
                                    cff_allocator_t allocator) {
  if (container.buffer == 0)
    return CFF_ERR_INVALID_OPERATION;

  cff_allocator_release(&allocator, (void *)(container.buffer));

  return CFF_ERR_NONE;
}

cff_err_e cff_ord_container_resize(cff_ord_container_s *container,
                                   uint64_t new_capacity,
                                   cff_allocator_t allocator) {
  cff_size new_size = container->data_size * (cff_size)new_capacity;
  void *old_buffer = (void *)container->buffer;
  void *new_buffer = cff_allocator_reallocate(&allocator, old_buffer, new_size);

  if (new_buffer == NULL)
    return CFF_ERR_REALLOC;

  container->buffer = (uintptr_t)new_buffer;
  container->capacity = new_capacity;

  return CFF_ERR_NONE;
}

bool cff_ord_container_is_valid(cff_ord_container_s container) {
  return container.error_code == CFF_ERR_NONE;
}

cff_iterator_s cff_ord_container_get_iterator(cff_ord_container_s *container) {
  cff_iterator_s it = {
      .index = 0,
      .current_item = 0,
      .data = (cff_container_s *)container,
      .interf =
          (cff_container_iterator_i){
              .get_capacity = cff_ord_container_get_capacity,
              .get_count = cff_ord_container_get_capacity,
              .get_data_size = cff_ord_container_get_data_size,
              .get_item_ref = cff_ord_container_get_item_ref,
          },
  };

  cff_iterator_reset(&it);

  return it;
}

cff_ord_container_s cff_ord_container_copy(cff_ord_container_s container,
                                           cff_allocator_t allocator) {
  cff_ord_container_s new_container = cff_ord_container_create(
      container.data_size, container.capacity, container.comparer, allocator);

  if (!cff_ord_container_is_valid(new_container))
    return new_container;

  const void *from = (const void *)(container.buffer);
  void *dest = (void *)(new_container.buffer);
  cff_size size = (cff_size)(container.capacity * container.data_size);

  cff_mem_copy(from, dest, size);

  return new_container;
}

bool cff_ord_container_equals(cff_ord_container_s container,
                              cff_ord_container_s other) {
  cff_size size = (cff_size)(container.capacity * container.data_size);

  const void *const a = (const void *const)container.buffer;
  const void *const b = (const void *const)other.buffer;

  return cff_mem_cmp(a, b, size);
}

bool cff_ord_container_contains(cff_ord_container_s container,
                                uintptr_t value) {
  const void *const ref = (const void *const)value;

  for (uint64_t i = 0; i < container.capacity; i++) {
    const void *const from =
        (const void *const)(container.buffer + (i * container.data_size));
    if (cff_mem_cmp(from, ref, container.data_size))
      return true;
  }
  return false;
}

void cff_ord_container_fill(cff_ord_container_s container, uintptr_t value,
                            cff_size value_size) {
  const void *data = (const void *)value;
  void *dest = (void *)container.buffer;

  cff_size buffer_size = (cff_size)container.capacity * container.data_size;
  cff_mem_set(data, dest, value_size, buffer_size);
}

bool cff_ord_container_find_index(cff_ord_container_s container, uintptr_t in,
                                  uint64_t *out_index) {
  return cff_ordered_container_find(&container, in, out_index,
                                    container.comparer, container.capacity);
}