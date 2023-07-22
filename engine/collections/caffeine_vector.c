#include "caffeine_vector.h"
#include "caffeine_container_functions.h"

#define TO_CONTAINER_PTR(X) (*(cff_container_s *)(X))
#define TO_CONTAINER(X) (*(cff_container_s *)(&X))

#define TO_VECTOR_PTR(X) (*(cff_vector_s *)(X))
#define TO_VECTOR(X) (*(cff_vector_s *)(&X))

#define GROW_TAX 1.5

static inline cff_err_e cff_vector_resize(cff_vector_s *vector,
                                          uint64_t new_capacity,
                                          cff_allocator_t allocator) {

  cff_err_e err = cff_container_resize((cff_container_s *)(vector),
                                       new_capacity, allocator);
  if (err != CFF_ERR_NONE) {
    return err;
  }
  if (new_capacity < vector->count)
    vector->count = new_capacity;

  return CFF_ERR_NONE;
}

// TODO: handle errors
static inline cff_err_e cff_vector_grow(cff_vector_s *vector,
                                        cff_allocator_t allocator) {
  uint64_t new_capacity = (uint64_t)(((double)vector->count) * GROW_TAX);

  if (vector->count + 1 >= vector->capacity) {
    return cff_vector_resize(vector, new_capacity, allocator);
  }

  return CFF_ERR_NONE;
}

// TODO: handle errors
static inline cff_err_e cff_vector_shrink(cff_vector_s *vector,
                                          cff_allocator_t allocator) {
  if (vector->count <= vector->capacity / 2) {
    return cff_vector_resize(vector, vector->count, allocator);
  }

  return CFF_ERR_NONE;
}

cff_vector_s cff_vector_create(cff_size data_size, uint64_t capacity,
                               cff_allocator_t allocator) {
  cff_container_s container =
      cff_container_create(data_size, capacity, allocator);
  cff_vector_s vector = TO_VECTOR(container);
  vector.count = 0;
  return vector;
}

cff_iterator_s cff_vector_get_iterator(cff_vector_s *vector) {
  return cff_container_get_iterator((cff_container_s *)vector);
}

cff_vector_s cff_vector_copy(cff_vector_s vector, cff_allocator_t allocator) {
  cff_container_s container =
      cff_container_copy(TO_CONTAINER(vector), allocator);
  cff_vector_s out_vector = TO_VECTOR(container);
  out_vector.count = vector.count;
  return out_vector;
}

cff_err_e cff_vector_get(cff_vector_s vector, uint64_t index, uintptr_t out) {
  if (index >= vector.count)
    return CFF_ERR_OUT_OF_BOUNDS;
  return cff_container_get(TO_CONTAINER(vector), index, out);
}

cff_err_e cff_vector_get_ref(cff_vector_s vector, uint64_t index,
                             uintptr_t *out) {
  if (index >= vector.count)
    return CFF_ERR_OUT_OF_BOUNDS;
  return cff_container_get_ref(TO_CONTAINER(vector), index, out);
}

cff_err_e cff_vector_set(cff_vector_s vector, uint64_t index, uintptr_t in) {
  if (index >= vector.count)
    return CFF_ERR_OUT_OF_BOUNDS;
  cff_container_s c = TO_CONTAINER(vector);
  return cff_container_set(c, index, in);
}

cff_err_e cff_vector_insert(cff_vector_s *vector, uint64_t index, uintptr_t in,
                            cff_allocator_t allocator) {
  if (index > vector->count)
    return CFF_ERR_OUT_OF_BOUNDS;

  {
    cff_err_e err = cff_vector_grow(vector, allocator);
    if (err != CFF_ERR_NONE)
      return err;
  }

  {
    cff_err_e err =
        cff_container_insert(*(cff_container_s *)(vector), index, in);
    if (err != CFF_ERR_NONE)
      return err;
  }

  vector->count++;

  return CFF_ERR_NONE;
}

cff_err_e cff_vector_remove(cff_vector_s *vector, uint64_t index,
                            cff_allocator_t allocator) {
  if (index >= vector->count)
    return CFF_ERR_OUT_OF_BOUNDS;

  {
    cff_err_e err = cff_container_remove(*(cff_container_s *)(vector), index);
    if (err != CFF_ERR_NONE)
      return err;
  }

  vector->count--;

  return cff_vector_shrink(vector, allocator);
}

cff_err_e cff_vector_destroy(cff_vector_s vector, cff_allocator_t allocator) {
  return cff_container_destroy(TO_CONTAINER(vector), allocator);
}

bool cff_vector_is_valid(cff_vector_s vector) {
  return cff_container_is_valid(TO_CONTAINER(vector));
}

bool cff_vector_equals(cff_vector_s vector, cff_vector_s other) {
  return cff_container_equals(TO_CONTAINER(vector), TO_CONTAINER(other));
}

bool cff_vector_contains(cff_vector_s vector, uintptr_t value) {
  return cff_container_contains(TO_CONTAINER(vector), value);
}

void cff_vector_fill(cff_vector_s vector, uintptr_t value,
                     cff_size value_size) {
  cff_container_fill(TO_CONTAINER(vector), value, value_size);
}

cff_err_e cff_vector_sort(cff_vector_s vector, uint64_t start, uint64_t end,
                          cff_order_function order) {
  return cff_container_sort(TO_CONTAINER(vector), start, end, order);
}

cff_err_e cff_vector_reserve(cff_vector_s *vector, uint64_t capacity,
                             cff_allocator_t allocator) {
  uint64_t capacity_req = vector->count + capacity;

  if (capacity_req > vector->capacity) {
    uint64_t new_capacity = (uint64_t)((double)capacity_req * GROW_TAX);
    {
      cff_err_e err = cff_vector_resize(vector, new_capacity, allocator);
      if (err != CFF_ERR_NONE)
        return err;
    }
  }

  vector->count = capacity_req;

  return CFF_ERR_NONE;
}

cff_err_e cff_vector_push_back(cff_vector_s *vector, uintptr_t in,
                               cff_allocator_t allocator) {
  {
    cff_err_e err = cff_vector_grow(vector, allocator);
    if (err != CFF_ERR_NONE)
      return err;
  }

  {
    cff_err_e err =
        cff_container_set(TO_CONTAINER_PTR(vector), vector->count, in);
    if (err != CFF_ERR_NONE)
      return err;
  }

  vector->count++;

  return CFF_ERR_NONE;
}

cff_err_e cff_vector_pop_back(cff_vector_s *vector, cff_allocator_t allocator) {

  if (vector->count > 0) {
    vector->count--;
    return cff_vector_shrink(vector, allocator);
  }

  return CFF_ERR_INVALID_OPERATION;
}

cff_err_e cff_vector_push_front(cff_vector_s *vector, uintptr_t in,
                                cff_allocator_t allocator) {

  {
    cff_err_e err = cff_vector_grow(vector, allocator);
    if (err != CFF_ERR_NONE)
      return err;
  }

  {
    cff_err_e err = cff_container_insert(TO_CONTAINER_PTR(vector), 0, in);
    if (err != CFF_ERR_NONE)
      return err;
  }

  vector->count++;

  return CFF_ERR_NONE;
}

cff_err_e cff_vector_pop_front(cff_vector_s *vector,
                               cff_allocator_t allocator) {

  if (vector->count <= 0)
    return CFF_ERR_INVALID_OPERATION;

  {
    cff_err_e err = cff_container_remove(TO_CONTAINER_PTR(vector), 0);
    if (err != CFF_ERR_NONE)
      return err;
  }

  vector->count--;

  return cff_vector_shrink(vector, allocator);
}