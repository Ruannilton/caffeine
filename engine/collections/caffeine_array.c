#include "caffeine_array.h"
#include "caffeine_container_functions.h"

#define TO_CONTAINER_PTR(X) (*(cff_container_s *)(X))
#define TO_CONTAINER(X) (*(cff_container_s *)(&X))

#define TO_ARRAY_PTR(X) (*(cff_array_s *)(X))
#define TO_ARRAY(X) (*(cff_array_s *)(&X))

cff_array_s cff_array_create(cff_size data_size, uint64_t capacity,
                             cff_allocator_t allocator) {
  cff_container_s container =
      cff_container_create(data_size, capacity, allocator);
  cff_array_s array = TO_ARRAY(container);
  return array;
}

cff_iterator_s cff_array_get_iterator(cff_array_s *array) {
  return cff_container_get_iterator((cff_container_s *)array);
}

cff_array_s cff_array_copy(cff_array_s array, cff_allocator_t allocator) {
  cff_container_s container =
      cff_container_copy(TO_CONTAINER(array), allocator);
  cff_array_s out_array = TO_ARRAY(container);
  return out_array;
}

cff_err_e cff_array_get(cff_array_s array, uint64_t index, uintptr_t out) {
  if (index >= array.capacity)
    return CFF_ERR_OUT_OF_BOUNDS;
  return cff_container_get(TO_CONTAINER(array), index, out);
}

cff_err_e cff_array_get_ref(cff_array_s array, uint64_t index, uintptr_t *out) {
  if (index >= array.capacity)
    return CFF_ERR_OUT_OF_BOUNDS;
  return cff_container_get_ref(TO_CONTAINER(array), index, out);
}

cff_err_e cff_array_set(cff_array_s array, uint64_t index, uintptr_t in) {
  if (index >= array.capacity)
    return CFF_ERR_OUT_OF_BOUNDS;
  cff_container_s c = TO_CONTAINER(array);
  return cff_container_set(c, index, in);
}

cff_err_e cff_array_insert(cff_array_s *array, uint64_t index, uintptr_t in,
                           cff_allocator_t allocator) {
  if (index > array->capacity)
    return CFF_ERR_OUT_OF_BOUNDS;

  return cff_container_insert(*(cff_container_s *)(array), index, in);
}

cff_err_e cff_array_remove(cff_array_s *array, uint64_t index,
                           cff_allocator_t allocator) {
  if (index >= array->capacity)
    return CFF_ERR_OUT_OF_BOUNDS;

  return cff_container_remove(*(cff_container_s *)(array), index);
}

cff_err_e cff_array_destroy(cff_array_s array, cff_allocator_t allocator) {
  return cff_container_destroy(TO_CONTAINER(array), allocator);
}

bool cff_array_is_valid(cff_array_s array) {
  return cff_container_is_valid(TO_CONTAINER(array));
}

bool cff_array_equals(cff_array_s array, cff_array_s other) {
  return cff_container_equals(TO_CONTAINER(array), TO_CONTAINER(other));
}

bool cff_array_contains(cff_array_s array, uintptr_t value) {
  return cff_container_contains(TO_CONTAINER(array), value);
}

void cff_array_fill(cff_array_s array, uintptr_t value, cff_size value_size) {
  cff_container_fill(TO_CONTAINER(array), value, value_size);
}

cff_err_e cff_array_sort(cff_array_s array, uint64_t start, uint64_t end,
                         cff_order_function order) {
  return cff_container_sort(TO_CONTAINER(array), start, end, order);
}
