#include "caffeine_container_functions.h"

static uint64_t cff_container_get_capacity(void *ptr) {
  cff_container_s *container = (cff_container_s *)ptr;
  return container->capacity;
}

static cff_size cff_container_get_data_size(void *ptr) {
  cff_container_s *container = (cff_container_s *)ptr;
  return container->data_size;
}

static bool cff_container_get_item_ref(void *ptr, uint64_t index,
                                       uintptr_t *out) {
  cff_container_s *container = (cff_container_s *)ptr;
  return cff_container_get_ref(*container, index, out);
}

cff_container_s cff_container_create(cff_size data_size, uint64_t capacity,
                                     cff_allocator allocator) {

  uintptr_t buffer_address = 0;

  cff_err_e alloc_e = cff_allocator_alloc(
      allocator, (cff_size)(capacity * data_size), &buffer_address);

  if (alloc_e != CFF_ERR_NONE) {
    return (cff_container_s){.error_code = alloc_e};
  }

  return (cff_container_s){.data_size = data_size,
                           .capacity = capacity,
                           .error_code = CFF_ERR_NONE,
                           .buffer = buffer_address};
}

cff_err_e cff_container_get(cff_container_s container, uint64_t index,
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

cff_err_e cff_container_get_ref(cff_container_s container, uint64_t index,
                                uintptr_t *out) {
  debug_assert((uintptr_t)out != 0);

  if (index >= container.capacity)
    return CFF_ERR_OUT_OF_BOUNDS;

  uintptr_t ref = (uintptr_t)(container.buffer + (index * container.data_size));

  *out = ref;

  return CFF_ERR_NONE;
}

cff_err_e cff_container_set(cff_container_s container, uint64_t index,
                            uintptr_t in) {
  debug_assert(in != 0);

  if (index >= container.capacity)
    return CFF_ERR_OUT_OF_BOUNDS;

  void *to = (void *)(container.buffer + (index * container.data_size));
  const void *from = (const void *)in;

  cff_mem_copy(from, to, container.data_size);

  return CFF_ERR_NONE;
}

cff_err_e cff_container_insert(cff_container_s container, uint64_t index,
                               uintptr_t in) {
  debug_assert(in != 0);

  if (index >= container.capacity)
    return CFF_ERR_OUT_OF_BOUNDS;

  uint64_t payload_size =
      ((container.capacity - 1) - index) * container.data_size;

  {
    const void *from =
        (const void *)(container.buffer + index * container.data_size);
    void *to = (void *)(container.buffer + (index + 1) * container.data_size);

    cff_mem_move(from, to, (cff_size)payload_size);
  }

  {
    void *to = (void *)(container.buffer + (index * container.data_size));
    const void *from = (const void *)in;

    cff_mem_copy(from, to, container.data_size);
  }

  return CFF_ERR_NONE;
}

cff_err_e cff_container_remove(cff_container_s container, uint64_t index) {
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

cff_err_e cff_container_destroy(cff_container_s container,
                                cff_allocator allocator) {
  if (container.buffer == 0)
    return CFF_ERR_INVALID_OPERATION;

  cff_allocator_release(allocator, container.buffer);

  return CFF_ERR_NONE;
}

cff_err_e cff_container_resize(cff_container_s *container,
                               uint64_t new_capacity, cff_allocator allocator) {

  cff_size new_size = container->data_size * (cff_size)new_capacity;

  uintptr_t old_buffer = container->buffer;

  uintptr_t new_buffer = 0;

  cff_err_e new_buffer_error =
      cff_allocator_realloc(allocator, old_buffer, new_size, &new_buffer);

  if (IS_ERROR(new_buffer_error))
    return new_buffer_error;

  container->buffer = new_buffer;
  container->capacity = new_capacity;

  return CFF_ERR_NONE;
}

bool cff_container_is_valid(cff_container_s container) {
  return container.error_code == CFF_ERR_NONE;
}

cff_iterator_s cff_container_get_iterator(cff_container_s *container) {
  cff_iterator_s it = {
      .index = 0,
      .current_item = 0,
      .data = container,
      .interf =
          (cff_container_iterator_i){
              .get_capacity = cff_container_get_capacity,
              .get_count = cff_container_get_capacity,
              .get_data_size = cff_container_get_data_size,
              .get_item_ref = cff_container_get_item_ref,
          },
  };

  cff_iterator_reset(&it);

  return it;
}

cff_container_s cff_container_copy(cff_container_s container,
                                   cff_allocator allocator) {
  cff_container_s new_container =
      cff_container_create(container.data_size, container.capacity, allocator);

  if (!cff_container_is_valid(new_container))
    return new_container;

  const void *from = (const void *)(container.buffer);
  void *dest = (void *)(new_container.buffer);
  cff_size size = (cff_size)(container.capacity * container.data_size);

  cff_mem_copy(from, dest, size);

  return new_container;
}

bool cff_container_equals(cff_container_s container, cff_container_s other) {
  cff_size size = (cff_size)(container.capacity * container.data_size);

  const void *const a = (const void *const)container.buffer;
  const void *const b = (const void *const)other.buffer;

  return cff_mem_cmp(a, b, size);
}

bool cff_container_contains(cff_container_s container, uintptr_t value) {
  const void *const ref = (const void *const)value;

  for (uint64_t i = 0; i < container.capacity; i++) {
    const void *const from =
        (const void *const)(container.buffer + (i * container.data_size));
    if (cff_mem_cmp(from, ref, container.data_size))
      return true;
  }
  return false;
}

void cff_container_fill(cff_container_s container, uintptr_t value,
                        cff_size value_size) {
  const void *data = (const void *)value;
  void *dest = (void *)container.buffer;

  cff_size buffer_size = (cff_size)container.capacity * container.data_size;
  cff_mem_set(data, dest, value_size, buffer_size);
}

cff_err_e cff_container_sort(cff_container_s container, uint64_t start,
                             uint64_t end, cff_order_function order) {
  debug_assert(end < container.capacity);
  debug_assert(start <= end);
  return cff_quick_sort(container.buffer, container.data_size, order, start,
                        end);
}