#pragma once

#include "../caffeine_memory.h"

#define HEADER_SIZE sizeof(struct cff_block_t)

#define SKIP_HEADER(ptr) (((uintptr_t)(ptr)) + HEADER_SIZE)

#define GET_HEADER(ptr)                                                        \
  ((struct cff_block_t *)(((uintptr_t)(ptr)) - HEADER_SIZE))

#define GET_SIZE(ptr) ((ptr)->block_size)

#define SET_SIZE(ptr, size) ((ptr)->block_size = size)

struct cff_block_t {
  uint32_t block_size;
};

struct cff_allocator_t {
  void *context; /**< The context associated with the allocator. */
  cff_err_e (*allocate)(
      void *context, cff_size size,
      uintptr_t *out); /**< Function pointer for memory allocation. */
  cff_err_e (*release)(
      void *context,
      uintptr_t ptr); /**< Function pointer for releasing allocated memory. */
  cff_err_e (*reallocate)(
      void *context, uintptr_t ptr, cff_size size,
      uintptr_t *out); /**< Function pointer for reallocating memory. */
  cff_err_e (*get_size)(void *context, uintptr_t ptr,
                        cff_size *out); /**< Function pointer for retrieving the
                            size of allocated memory. */
};