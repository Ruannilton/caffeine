#include "caffeine_platform.h"
#include "caffeine_types.h"

static inline cff_err_e __cff_quick_sort_swap(uintptr_t a, uintptr_t b,
                                              cff_size data_size);
static inline cff_err_e __cff_quick_sort_partition(uintptr_t arr,
                                                   cff_size data_size,
                                                   int64_t l, int64_t h,
                                                   cff_order_function predicate,
                                                   int64_t *pivot);

uint64_t cff_generic_hash(uintptr_t data_ptr, cff_size data_size,
                          uint64_t seed) {
  char *key = (char *)data_ptr;
  uint64_t h = (525201411107845655ull) + seed;

  for (; data_size; --data_size, ++key) {
    h ^= *key;
    h *= 0x5bd1e9955bd1e995;
    h ^= h >> 47;
  }

  return h;
}

bool cff_binary_search(uintptr_t arr_ptr, uintptr_t value_ptr, uint64_t start,
                       uint64_t lenght, cff_size data_size,
                       cff_order_function cmp_fn, uint64_t *out_index) {
  uint64_t low = start;
  uint64_t hi = (start + lenght - 1);

  while (low <= hi) {
    uint64_t i = (uint64_t)(low + ((hi - low) >> 1));

    uintptr_t cmp_ptr = arr_ptr + (uintptr_t)(i * data_size);
    cff_ord_e cmp_result = cmp_fn(cmp_ptr, value_ptr, data_size);

    if (cmp_result == CFF_EQUAL) {
      if (out_index != NULL)
        *out_index = i;
      return true;
    }
    if (cmp_result == CFF_LESS)
      low = i + 1;
    else {
      if (i == 0)
        break;
      hi = i - 1;
    }
  }

  if (out_index != NULL)
    *out_index = low;
  return false;
}

cff_err_e cff_quick_sort(uintptr_t buffer, cff_size data_size,
                         cff_order_function predicate, uint64_t left,
                         uint64_t right) {

  cff_size stack_size = (size_t)(sizeof(uint64_t) * (right - left + 1));
  int64_t *stack = NULL;

  defer(stack = (int64_t *)cff_stack_alloc(stack_size),
        cff_stack_free((void *)stack)) {
    if (stack == NULL)
      return CFF_ERR_ALLOC;

    int64_t top = -1;
    int64_t h = 0;
    int64_t l = 0;

    // push left and right to stack
    stack[++top] =
        (int64_t)left; // cff_mem_copy((const void*)&left,
                       // (void*)(stack + (++top)), sizeof(uint64_t));
    stack[++top] =
        (int64_t)right; // cff_mem_copy((const void*)&right, (void*)(stack +
                        // (++top)), sizeof(uint64_t));

    while (top >= 0) {
      h = stack[top--]; //	cff_mem_copy((const void*)(stack + (top--)),
                        //(void*)&h, sizeof(uint64_t));
      l = stack[top--]; // cff_mem_copy((const void*)(stack + (top--)),
                        // (void*)&l, sizeof(uint64_t));

      debug_assert(l >= left);
      debug_assert(h <= right);

      int64_t pivot = 0;
      cff_err_e err = __cff_quick_sort_partition(buffer, data_size, l, h,
                                                 predicate, &pivot);
      if (err != CFF_ERR_NONE)
        return err;

      int64_t tmp = pivot - 1;

      if (tmp > l) {
        stack[++top] = l;   // cff_mem_copy((const void*)&l, (void*)(stack +
                            // (++top)), sizeof(uint64_t));
        stack[++top] = tmp; // cff_mem_copy((const void*)&tmp, (void*)(stack +
                            // (++top)), sizeof(uint64_t));
      }

      tmp = pivot + 1;
      if (tmp < h) {
        stack[++top] = tmp; // cff_mem_copy((const void*)&tmp, (void*)(stack +
                            // (++top)), sizeof(uint64_t));
        stack[++top] = h;   // cff_mem_copy((const void*)&h, (void*)(stack +
                            // (++top)), sizeof(uint64_t));
      }
    }
  }

  return CFF_ERR_NONE;
}

static inline cff_err_e __cff_quick_sort_swap(uintptr_t a, uintptr_t b,
                                              cff_size data_size) {
  uintptr_t tmp = 0;

  defer(tmp = (uintptr_t)cff_stack_alloc(data_size),
        cff_stack_free((void *)tmp)) {
    if (tmp == 0)
      return CFF_ERR_ALLOC;
    cff_mem_copy((const void *)a, (void *)tmp, data_size);
    cff_mem_copy((const void *)b, (void *)a, data_size);
    cff_mem_copy((const void *)tmp, (void *)b, data_size);
  }

  return CFF_ERR_NONE;
}

static inline cff_err_e __cff_quick_sort_partition(uintptr_t arr,
                                                   cff_size data_size,
                                                   int64_t left, int64_t right,
                                                   cff_order_function predicate,
                                                   int64_t *pivot) {
  debug_assert(left >= 0);
  debug_assert(right > 0);
  uintptr_t x = arr + (uintptr_t)(right * data_size);

  int64_t i = left - 1;

  for (int64_t j = left; j <= right - 1; j++) {
    if (predicate((uintptr_t)(arr + (j * data_size)), (uintptr_t)x,
                  data_size) == CFF_LESS) {
      i++;
      __cff_quick_sort_swap((uintptr_t)(arr + (i * data_size)),
                            (uintptr_t)(arr + (j * data_size)), data_size);
    }
  }

  cff_err_e err =
      __cff_quick_sort_swap((uintptr_t)(arr + ((i + 1) * data_size)),
                            (uintptr_t)(arr + (right * data_size)), data_size);

  if (err != CFF_ERR_NONE)
    return err;

  *pivot = i + 1;
  return CFF_ERR_NONE;
}