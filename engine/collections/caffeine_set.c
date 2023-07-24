#include "caffeine_set.h"

static cff_set_s *_set_resize(cff_set_s *set, uint64_t new_size,
                              cff_allocator allocator) {

  uintptr_t data_buffer = 0;
  cff_err_e dt_buffer_err = cff_allocator_alloc(
      allocator, (cff_size)(new_size * set->data_size), &data_buffer);

  if (IS_ERROR(dt_buffer_err)) {
    set->error_code = dt_buffer_err;
    return set;
  }

  bool *used_buffer = NULL;

  cff_err_e used_err =
      cff_allocator_alloc(allocator, (cff_size)(new_size * sizeof(bool)),
                          (uintptr_t *)(&used_buffer));

  if (IS_ERROR(used_err)) {
    set->error_code = used_err;
    cff_allocator_release(allocator, data_buffer);
    return set;
  }

  uint64_t max_collision_count = 0;

  for (size_t i = 0; i < set->capacity; i++) {
    uintptr_t value = (uintptr_t)set->data * (uintptr_t)(set->data_size * i);

    uint64_t collision_count = 0;
    uint64_t entry_index = set->hash_fn(value, set->data_size, 0) % new_size;

    while (used_buffer[entry_index]) {
      collision_count++;
      entry_index =
          set->hash_fn(value, set->data_size, collision_count) % new_size;
    }

    if (max_collision_count < collision_count)
      max_collision_count = collision_count;

    used_buffer[entry_index] = true;

    uintptr_t current_data_slot =
        ((uintptr_t)data_buffer) + (uintptr_t)(set->data_size * entry_index);

    cff_mem_copy((const void *)value, (void *)current_data_slot,
                 set->data_size);
  }

  cff_allocator_release(allocator, (uintptr_t)set->data);
  cff_allocator_release(allocator, (uintptr_t)set->used_slot);

  set->collision_count = max_collision_count;
  set->data = (void *)data_buffer;
  set->used_slot = used_buffer;

  return set;
}

static uint64_t cff_set_get_capacity(void *ptr) {
  cff_set_s *container = (cff_set_s *)ptr;
  return container->capacity;
}

static cff_size cff_set_get_data_size(void *ptr) {
  cff_set_s *container = (cff_set_s *)ptr;
  return container->data_size;
}

static bool cff_set_get_item_ref(void *ptr, uint64_t index, uintptr_t *out) {
  cff_set_s *container = (cff_set_s *)ptr;
  if (!container->used_slot[index])
    return false;

  *out = (uintptr_t)container->data + (uintptr_t)(container->data_size * index);
  return true;
}

cff_set_s cff_set_create(cff_size data_size, uint64_t capacity,
                         cff_comparer_function comparer_fn,
                         cff_hash_function hash_fn, cff_allocator allocator) {

  uintptr_t data = 0;
  cff_err_e dt_error =
      cff_allocator_alloc(allocator, data_size * capacity, &data);

  if (IS_ERROR(dt_error)) {
    return (cff_set_s){.error_code = dt_error};
  }

  return (cff_set_s){.capacity = capacity,
                     .data_cmp_fn = comparer_fn,
                     .hash_fn = hash_fn,
                     .error_code = CFF_ERR_NONE,
                     .data_size = data_size,
                     .data = (void *)data,
                     .count = 0,
                     .collision_count = 0};
}

bool cff_set_add(cff_set_s *set, uintptr_t value, cff_allocator allocator) {

  if ((float)set->count / set->capacity >= 0.75f) {
    set = _set_resize(set, set->capacity * 2, allocator);
    if (set->error_code != CFF_ERR_NONE)
      return false;
  }

  uint64_t collision_count = 0;
  uint64_t entry_index = set->hash_fn(value, set->data_size, 0) % set->capacity;

  while (set->used_slot[entry_index]) {
    uintptr_t current_key =
        (uintptr_t)set->data + (uintptr_t)(collision_count * set->data_size);

    if (set->data_cmp_fn(value, current_key, set->data_size))
      continue;

    collision_count++;
    entry_index =
        set->hash_fn(value, set->data_size, collision_count) % set->capacity;
  }

  if (set->collision_count < collision_count)
    set->collision_count = collision_count;

  set->used_slot[entry_index] = true;

  uintptr_t current_data_slot =
      ((uintptr_t)set->data) + (uintptr_t)(set->data_size * entry_index);

  cff_mem_copy((const void *)value, (void *)current_data_slot, set->data_size);

  set->count++;
  return true;
}

bool cff_set_remove(cff_set_s *set, uintptr_t value, cff_allocator allocator) {
  uint64_t collision = 0;

  uint64_t entry_index =
      set->hash_fn(value, set->data_size, collision) % set->capacity;
  uintptr_t index_data = (uintptr_t)(set->data) +
                         (uintptr_t)((uint64_t)set->data_size * entry_index);

  while (!set->used_slot[entry_index] &&
         set->data_cmp_fn(value, index_data, set->data_size)) {
    collision++;

    if (collision > set->collision_count)
      return false;

    entry_index =
        set->hash_fn(value, set->data_size, collision) % set->capacity;
  }

  set->used_slot[entry_index] = false;
  set->count--;
  return true;
}

bool cff_set_get_index(cff_set_s set, uintptr_t value, uint64_t *index) {
  uint64_t collision = 0;

  uint64_t entry_index =
      set.hash_fn(value, set.data_size, collision) % set.capacity;
  uintptr_t index_data =
      (uintptr_t)(set.data) + (uintptr_t)(set.data_size * entry_index);

  while (!set.used_slot[entry_index] &&
         set.data_cmp_fn(value, index_data, set.data_size)) {
    collision++;

    if (collision > set.collision_count)
      return false;

    entry_index = set.hash_fn(value, set.data_size, collision) % set.capacity;
  }

  *index = entry_index;
  return true;
}

bool cff_set_get_contains(cff_set_s set, uintptr_t value) {
  uint64_t index = 0;
  return cff_set_get_index(set, value, &index);
}

void cff_set_clear(cff_set_s *set) {
  cff_mem_zero(set->used_slot, sizeof(bool), sizeof(bool) * set->count);
  set->count = 0;
  set->collision_count = 0;
}

void cff_set_destroy(cff_set_s *set, cff_allocator allocator) {
  cff_allocator_release(allocator, (uintptr_t)set->used_slot);
  cff_allocator_release(allocator, (uintptr_t)set->data);
  *set = (cff_set_s){0};
}

cff_set_s cff_set_copy(cff_set_s set, cff_allocator allocator) {
  uint64_t new_size = set.count * 2;

  cff_set_s new_set = cff_set_create(set.data_size, new_size, set.data_cmp_fn,
                                     set.hash_fn, allocator);

  cff_iterator_s it = cff_set_get_iterator(&set);

  while (it.current_item) {
    uintptr_t value = cff_iterator_current(&it);
    cff_set_add(&new_set, value, allocator);
    cff_iterator_next(&it);
  }
  return new_set;
}

cff_set_s cff_set_clone(cff_set_s set, cff_allocator allocator) {
  cff_set_s new_set = cff_set_create(set.data_size, set.capacity,
                                     set.data_cmp_fn, set.hash_fn, allocator);

  set.collision_count = set.collision_count;
  new_set.count = new_set.count;

  cff_mem_copy(set.data, new_set.data,
               (cff_size)(set.data_size * set.capacity));
  cff_mem_copy(set.used_slot, new_set.used_slot,
               (cff_size)(sizeof(bool) * set.capacity));

  return new_set;
}

cff_iterator_s cff_set_get_iterator(cff_set_s *set) {
  cff_iterator_s it = {
      .index = 0,
      .current_item = 0,
      .data = set,
      .interf =
          (cff_container_iterator_i){
              .get_capacity = cff_set_get_capacity,
              .get_count = cff_set_get_capacity,
              .get_data_size = cff_set_get_data_size,
              .get_item_ref = cff_set_get_item_ref,
          },
  };

  cff_iterator_reset(&it);

  return it;
}