#pragma once
#include "caffeine_core.h"
#include "caffeine_platform.h"
/**
 * @struct cff_allocator_t
 * @brief Represents a memory allocator with function pointers for memory
 * management.
 */
typedef struct {
  void *context; /**< The context associated with the allocator. */
  void *(*allocate)(
      void *context,
      cff_size size); /**< Function pointer for memory allocation. */
  void (*release)(
      void *context,
      void *ptr); /**< Function pointer for releasing allocated memory. */
  void *(*reallocate)(
      void *context, void *ptr,
      cff_size size); /**< Function pointer for reallocating memory. */
  cff_size (*get_size)(void *context,
                       void *ptr); /**< Function pointer for retrieving the size
                                      of allocated memory. */
} cff_allocator_t;

extern cff_allocator_t cff_global_allocator;

/**
 * @brief Allocates memory using the specified allocator.
 *
 * @param alloc The allocator to use for memory allocation.
 * @param size The size of the memory to allocate.
 * @return A pointer to the allocated memory.
 */
inline void *cff_allocator_allocate(cff_allocator_t *alloc, cff_size size) {
  return alloc->allocate(alloc->context, size);
}

/**
 * @brief Releases memory using the specified allocator.
 *
 * @param alloc The allocator to use for memory release.
 * @param ptr The pointer to the memory to release.
 */
inline void cff_allocator_release(cff_allocator_t *alloc, void *ptr) {
  alloc->release(alloc->context, ptr);
}

/**
 * @brief Reallocates memory using the specified allocator.
 *
 * @param alloc The allocator to use for memory reallocation.
 * @param ptr The pointer to the memory to reallocate.
 * @param size The new size of the memory.
 * @return A pointer to the reallocated memory.
 */
inline void *cff_allocator_reallocate(cff_allocator_t *alloc, void *ptr,
                                      cff_size size) {
  return alloc->reallocate(alloc->context, ptr, size);
}

/**
 * @brief Retrieves the size of allocated memory using the specified allocator.
 *
 * @param alloc The allocator to use for retrieving the size.
 * @param ptr The pointer to the allocated memory.
 * @return The size of the allocated memory.
 */
inline cff_size cff_allocator_get_size(cff_allocator_t *alloc, void *ptr) {
  return alloc->get_size(alloc->context, ptr);
}
