#include "caffeine_ordered_vector.h"
#include "private/caffeine_ord_container_functions.h"

#define TO_CONTAINER_PTR(X) (*(cff_ord_container_s *)(X))
#define TO_CONTAINER(X) (*(cff_ord_container_s *)(&X))

#define TO_VECTOR_PTR(X) (*(cff_ord_vector_s *)(X))
#define TO_VECTOR(X) (*(cff_ord_vector_s *)(&X))

#define GROW_TAX 1.5

static inline cff_err_e cff_ord_vector_resize(cff_ord_vector_s *vector,
                                              uint64_t new_capacity,
                                              cff_allocator_t allocator) {

  cff_err_e err = cff_ord_container_resize((cff_ord_container_s *)(vector),
                                           new_capacity, allocator);
  if (err != CFF_ERR_NONE)
    return err;

  if (new_capacity < vector->count)
    vector->count = new_capacity;

  return CFF_ERR_NONE;
}

static inline cff_err_e cff_ord_vector_shrink(cff_ord_vector_s *vector,
                                              cff_allocator_t allocator) {
  if (vector->count <= vector->capacity / 2) {
    return cff_ord_vector_resize(vector, vector->count, allocator);
  }
  return CFF_ERR_NONE;
}

cff_ord_vector_s cff_ord_vector_create(
    cff_size data_size, uint64_t capacity,
    cff_ord_e (*comparer_fn)(uintptr_t a, uintptr_t b, cff_size data_size),
    cff_allocator_t allocator) {
  cff_ord_container_s container =
      cff_ord_container_create(data_size, capacity, comparer_fn, allocator);
  cff_ord_vector_s vector = TO_VECTOR(container);
  vector.count = 0;
  return vector;
}

cff_iterator_s cff_ord_vector_get_iterator(cff_ord_vector_s *vector) {
  return cff_ord_container_get_iterator((cff_ord_container_s *)vector);
}

cff_ord_vector_s cff_ord_vector_copy(cff_ord_vector_s vector,
                                     cff_allocator_t allocator) {
  cff_ord_container_s container =
      cff_ord_container_copy(TO_CONTAINER(vector), allocator);
  cff_ord_vector_s out_vector = TO_VECTOR(container);
  out_vector.count = vector.count;
  return out_vector;
}

cff_err_e cff_ord_vector_get(cff_ord_vector_s vector, uint64_t index,
                             uintptr_t out) {
  if (index >= vector.count)
    return CFF_ERR_OUT_OF_BOUNDS;
  return cff_ord_container_get(TO_CONTAINER(vector), index, out);
}

cff_err_e cff_ord_vector_get_ref(cff_ord_vector_s vector, uint64_t index,
                                 uintptr_t *out) {
  if (index >= vector.count)
    return CFF_ERR_OUT_OF_BOUNDS;
  return cff_ord_container_get_ref(TO_CONTAINER(vector), index, out);
}

cff_err_e cff_ord_vector_add(cff_ord_vector_s *vector, uintptr_t in,
                             uint64_t *out_index, cff_allocator_t allocator) {
  if (in >= vector->count)
    return CFF_ERR_OUT_OF_BOUNDS;
  cff_ord_container_s c = TO_CONTAINER(vector);

  return cff_ord_container_add(c, in, out_index);
}

cff_err_e cff_ord_vector_remove(cff_ord_vector_s *vector, uint64_t index,
                                cff_allocator_t allocator) {
  if (index >= vector->count)
    return CFF_ERR_OUT_OF_BOUNDS;

  cff_err_e err =
      cff_ord_container_remove(*(cff_ord_container_s *)(vector), index);

  if (err != CFF_ERR_NONE) {
    return err;
  }

  vector->count--;

  return cff_ord_vector_shrink(vector, allocator);
}

cff_err_e cff_ord_vector_destroy(cff_ord_vector_s vector,
                                 cff_allocator_t allocator) {
  return cff_ord_container_destroy(TO_CONTAINER(vector), allocator);
}

bool cff_ord_vector_is_valid(cff_ord_vector_s vector) {
  return cff_ord_container_is_valid(TO_CONTAINER(vector));
}

bool cff_ord_vector_equals(cff_ord_vector_s vector, cff_ord_vector_s other) {
  return cff_ord_container_equals(TO_CONTAINER(vector), TO_CONTAINER(other));
}

bool cff_ord_vector_contains(cff_ord_vector_s vector, uintptr_t value) {
  return cff_ord_container_contains(TO_CONTAINER(vector), value);
}

void cff_ord_vector_fill(cff_ord_vector_s vector, uintptr_t value,
                         cff_size value_size) {
  cff_ord_container_fill(TO_CONTAINER(vector), value, value_size);
}

cff_err_e cff_ord_vector_reserve(cff_ord_vector_s *vector, uint64_t capacity,
                                 cff_allocator_t allocator) {
  uint64_t capacity_req = vector->count + capacity;

  if (capacity_req > vector->capacity) {
    uint64_t new_capacity = (uint64_t)((double)capacity_req * GROW_TAX);
    cff_err_e err = cff_ord_vector_resize(vector, new_capacity, allocator);
    if (err != CFF_ERR_NONE)
      return err;
  }

  vector->count = capacity_req;

  return CFF_ERR_NONE;
}
