#include "caffeine_linked_list.h"

static inline cff_linked_list_bucket *_get_bucket(cff_linked_list_s list,
                                                  uint64_t index) {
  cff_linked_list_bucket *aux = (cff_linked_list_bucket *)list.buffer;

  while (index--) {
    aux = aux->next;
  }

  return aux;
}

static inline cff_linked_list_bucket *
_new_bucket(cff_size data_size, uintptr_t data, cff_allocator_t allocator) {
  cff_linked_list_bucket *bucket = cff_allocator_allocate(
      &allocator, data_size + (cff_size)sizeof(cff_linked_list_bucket));
  uintptr_t data_pointer =
      ((uintptr_t)bucket) + (uintptr_t)sizeof(cff_linked_list_bucket);

  bucket->next = NULL;
  bucket->data = data_pointer;

  return bucket;
}

static inline uintptr_t _get_bucket_data(cff_linked_list_bucket *bucket) {
  return bucket->data;
}

static inline uintptr_t _set_bucket_data(cff_linked_list_bucket *bucket,
                                         uint64_t data_size, uintptr_t data) {
  cff_mem_copy((const void *)data, (void *)(bucket->data), data_size);
  return bucket->data;
}

static inline bool _cmp_bucket(cff_linked_list_bucket *a,
                               cff_linked_list_bucket *b, uint64_t data_size) {
  return cff_mem_cmp((const void *)a->data, (void *)b->data, data_size);
}

static inline void _free_bucket(cff_linked_list_bucket *bucket,
                                cff_allocator_t allocator) {
  cff_allocator_release(&allocator, (void *)bucket);
}

static inline cff_linked_list_bucket *_sorted_merge(cff_linked_list_bucket *a,
                                                    cff_linked_list_bucket *b,
                                                    cff_order_function order,
                                                    uint64_t data_size) {
  cff_linked_list_bucket *result = NULL;

  /* Base cases */
  if (a == NULL)
    return (b);
  else if (b == NULL)
    return (a);

  /* Pick either a or b, and recur */
  cff_ord_e eq = order(a->data, b->data, data_size);
  if (eq == CFF_EQUAL || eq == CFF_LESS) {
    result = a;
    result->next = _sorted_merge(a->next, b, order, data_size);
  } else {
    result = b;
    result->next = _sorted_merge(a, b->next, order, data_size);
  }
  return (result);
}

static inline void _front_back_split(cff_linked_list_bucket *source,
                                     cff_linked_list_bucket **frontRef,
                                     cff_linked_list_bucket **backRef) {
  cff_linked_list_bucket *fast;
  cff_linked_list_bucket *slow;
  slow = source;
  fast = source->next;

  /* Advance 'fast' two nodes, and advance 'slow' one node */
  while (fast != NULL) {
    fast = fast->next;
    if (fast != NULL) {
      slow = slow->next;
      fast = fast->next;
    }
  }

  /* 'slow' is before the midpoint in the list, so split it in two
  at that point. */
  *frontRef = source;
  *backRef = slow->next;
  slow->next = NULL;
}

static inline void _merge_sort(cff_linked_list_bucket **headRef,
                               cff_order_function order, uint64_t data_size) {
  cff_linked_list_bucket *head = *headRef;
  cff_linked_list_bucket *a;
  cff_linked_list_bucket *b;

  /* Base case -- length 0 or 1 */
  if ((head == NULL) || (head->next == NULL)) {
    return;
  }

  /* Split head into 'a' and 'b' sublists */
  _front_back_split(head, &a, &b);

  /* Recursively sort the sublists */
  _merge_sort(&a, order, data_size);
  _merge_sort(&b, order, data_size);

  /* answer = merge the two sorted lists together */
  *headRef = _sorted_merge(a, b, order, data_size);
}

static uint64_t _get_count(void *ptr) {
  cff_linked_list_s *linked_list = (cff_linked_list_s *)ptr;
  return linked_list->count;
}

static cff_size _get_data_size(void *ptr) {
  cff_linked_list_s *linked_list = (cff_linked_list_s *)ptr;
  return linked_list->data_size;
}

static bool _get_item_ref(void *ptr, uint64_t index, uintptr_t *out) {
  cff_linked_list_s *linked_list = (cff_linked_list_s *)ptr;
  if (index >= linked_list->count)
    return false;
  return cff_linked_list_get_ref(*linked_list, index, out) == CFF_ERR_NONE;
}

//

cff_linked_list_s cff_linked_list_create(cff_size data_size) {
  cff_linked_list_s linked = (cff_linked_list_s){
      .buffer = 0,
      .count = 0,
      .data_size = data_size,
      .error_code = CFF_ERR_NONE,
  };

  return linked;
}

cff_iterator_s cff_linked_list_get_iterator(cff_linked_list_s *linked_list) {
  cff_iterator_s it = (cff_iterator_s){.index = 0,
                                       .current_item = 0,
                                       .data = (void *)linked_list,
                                       .interf = (cff_container_iterator_i){
                                           .get_capacity = _get_count,
                                           .get_count = _get_count,
                                           .get_data_size = _get_data_size,
                                           .get_item_ref = _get_item_ref,
                                       }};
  return it;
}

cff_linked_list_s cff_linked_list_copy(cff_linked_list_s linked_list,
                                       cff_allocator_t allocator) {
  cff_linked_list_s list = cff_linked_list_create(linked_list.data_size);

  if (list.error_code != CFF_ERR_NONE)
    return list;

  cff_linked_list_bucket *aux = (cff_linked_list_bucket *)linked_list.buffer;
  while (aux != NULL) {
    cff_linked_list_push_back(&list, (uintptr_t)(aux->data), allocator);
    aux = aux->next;
  }

  return list;
}

cff_err_e cff_linked_list_get(cff_linked_list_s linked_list, uint64_t index,
                              uintptr_t out) {
  if (index >= linked_list.count)
    return CFF_ERR_OUT_OF_BOUNDS;

  cff_linked_list_bucket *aux = _get_bucket(linked_list, index);
  uintptr_t data = _get_bucket_data(aux);

  cff_mem_copy((const void *)data, (void *)out, linked_list.data_size);

  return CFF_ERR_NONE;
}

cff_err_e cff_linked_list_get_ref(cff_linked_list_s linked_list, uint64_t index,
                                  uintptr_t *out) {
  if (index >= linked_list.count)
    return CFF_ERR_OUT_OF_BOUNDS;

  cff_linked_list_bucket *aux = _get_bucket(linked_list, index);

  *out = _get_bucket_data(aux);

  return CFF_ERR_NONE;
}

cff_err_e cff_linked_list_set(cff_linked_list_s linked_list, uint64_t index,
                              uintptr_t in) {
  if (index >= linked_list.count)
    return CFF_ERR_OUT_OF_BOUNDS;

  cff_linked_list_bucket *aux = _get_bucket(linked_list, index);

  _set_bucket_data(aux, linked_list.data_size, in);

  return CFF_ERR_NONE;
}

cff_err_e cff_linked_list_insert(cff_linked_list_s *linked_list, uint64_t index,
                                 uintptr_t in, cff_allocator_t allocator) {
  if (index >= linked_list->count)
    return CFF_ERR_OUT_OF_BOUNDS;
  cff_linked_list_bucket *bucket =
      _new_bucket(linked_list->data_size, in, allocator);

  if (index == 0) {
    bucket->next = 0;
    linked_list->buffer = (uintptr_t)bucket;
    if (linked_list->count == 0)
      linked_list->last = bucket;
  } else if (index == linked_list->count) {
    linked_list->last->next = bucket;
    linked_list->last = bucket;
  } else {
    cff_linked_list_bucket *aux = _get_bucket(*linked_list, index - 1);
    bucket->next = aux->next;
    aux->next = bucket;
  }

  linked_list->count++;
  return CFF_ERR_NONE;
}

// TODO: handle errors
cff_err_e cff_linked_list_remove(cff_linked_list_s *linked_list, uint64_t index,
                                 cff_allocator_t allocator) {
  if (index == 0) {
    uintptr_t aux = linked_list->buffer;

    linked_list->buffer =
        (uintptr_t)((cff_linked_list_bucket *)linked_list->buffer)->next;
    _free_bucket((cff_linked_list_bucket *)aux, allocator);
  } else {
    cff_linked_list_bucket *aux = _get_bucket(*linked_list, index - 1);
    cff_linked_list_bucket *aux_next = aux->next;

    if (aux->next->next != NULL)
      aux->next = aux->next->next;

    _free_bucket(aux_next, allocator);
  }

  linked_list->count--;
  return CFF_ERR_NONE;
}

// TODO: handle errors
cff_err_e cff_linked_list_destroy(cff_linked_list_s linked_list,
                                  cff_allocator_t allocator) {
  while (linked_list.buffer != 0) {
    cff_linked_list_bucket *aux =
        (cff_linked_list_bucket *)(linked_list.buffer);
    linked_list.buffer =
        (uintptr_t)(((cff_linked_list_bucket *)linked_list.buffer)->next);
    _free_bucket(aux, allocator);
    linked_list.count--;
  }
  linked_list = (cff_linked_list_s){0};

  return CFF_ERR_NONE;
}
// TODO: handle errors
cff_err_e cff_linked_list_reserve(cff_linked_list_s *linked_list,
                                  uint64_t count, cff_allocator_t allocator) {
  while (linked_list->count < count) {
    cff_linked_list_push_back(linked_list, 0, allocator);
  }

  return CFF_ERR_NONE;
}

cff_err_e cff_linked_list_push_back(cff_linked_list_s *linked_list,
                                    uintptr_t in, cff_allocator_t allocator) {
  cff_linked_list_bucket *bucket =
      _new_bucket(linked_list->data_size, in, allocator);
  bucket->next = 0;

  if (linked_list->count == 0) {
    linked_list->buffer = (uintptr_t)bucket;
    linked_list->last = bucket;
  } else {
    linked_list->last->next = bucket;
    linked_list->last = bucket;
  }

  linked_list->count++;
  return CFF_ERR_NONE;
}

cff_err_e cff_linked_list_pop_back(cff_linked_list_s *linked_list,
                                   cff_allocator_t allocator) {
  if (linked_list->count == 0)
    return CFF_ERR_OUT_OF_BOUNDS;
  if (linked_list->count == 1) {
    _free_bucket((cff_linked_list_bucket *)(linked_list->buffer), allocator);
    linked_list->buffer = 0;
    linked_list->last = 0;
  }

  cff_linked_list_bucket *new_last =
      _get_bucket(*linked_list, linked_list->count - 1);
  _free_bucket(new_last->next, allocator);
  new_last->next = 0;
  linked_list->last = new_last;
  linked_list->count--;
  return CFF_ERR_NONE;
}

cff_err_e cff_linked_list_push_front(cff_linked_list_s *linked_list,
                                     uintptr_t in, cff_allocator_t allocator) {
  cff_linked_list_bucket *bucket =
      _new_bucket(linked_list->data_size, in, allocator);
  bucket->next = (cff_linked_list_bucket *)(linked_list->buffer);
  linked_list->buffer = (uintptr_t)bucket;
  linked_list->count++;
  return CFF_ERR_NONE;
}

cff_err_e cff_linked_list_pop_front(cff_linked_list_s *linked_list,
                                    cff_allocator_t allocator) {
  if (linked_list->count == 0)
    return CFF_ERR_OUT_OF_BOUNDS;

  cff_linked_list_bucket *aux = (cff_linked_list_bucket *)(linked_list->buffer);
  linked_list->buffer = (uintptr_t)(aux->next);
  linked_list->count--;

  _free_bucket(aux, allocator);

  return CFF_ERR_NONE;
}

bool cff_linked_list_is_valid(cff_linked_list_s linked_list) {
  return linked_list.error_code = CFF_ERR_NONE;
}

bool cff_linked_list_equals(cff_linked_list_s linked_list,
                            cff_linked_list_s other) {

  if (linked_list.count != other.count)
    return false;

  cff_linked_list_bucket *aux_a =
      (cff_linked_list_bucket *)(linked_list.buffer);
  cff_linked_list_bucket *aux_b = (cff_linked_list_bucket *)(other.buffer);

  while (aux_a != NULL && aux_b != NULL) {
    if (!_cmp_bucket(aux_a, aux_b, linked_list.data_size))
      return false;

    aux_a = aux_a->next;
    aux_b = aux_b->next;
  }

  return true;
}

bool cff_linked_list_contains(cff_linked_list_s linked_list, uintptr_t value) {
  cff_linked_list_bucket *aux_a =
      (cff_linked_list_bucket *)(linked_list.buffer);
  while (aux_a != NULL) {
    if (cff_mem_cmp((const void *const)value, aux_a, linked_list.data_size))
      return true;
    aux_a = aux_a->next;
  }
  return false;
}

void cff_linked_list_fill(cff_linked_list_s linked_list, uintptr_t value,
                          cff_size value_size) {
  cff_linked_list_bucket *aux_a =
      (cff_linked_list_bucket *)(linked_list.buffer);
  while (aux_a != NULL) {
    _set_bucket_data(aux_a, value_size, value);
    aux_a = aux_a->next;
  }
}

cff_err_e cff_linked_list_sort(cff_linked_list_s linked_list, uint64_t start,
                               uint64_t end, cff_order_function order) {
  if (linked_list.count == 0)
    return CFF_ERR_OUT_OF_BOUNDS;

  if (linked_list.count == 1)
    return CFF_ERR_NONE;

  cff_linked_list_bucket *head = (cff_linked_list_bucket *)(linked_list.buffer);

  _merge_sort(&head, order, linked_list.data_size);

  linked_list.buffer = (uintptr_t)head;

  return CFF_ERR_NONE;
}