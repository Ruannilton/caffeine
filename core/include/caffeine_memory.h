#pragma once
#include <caffeine_core.h>

/**
 * @struct cff_allocator_t
 * @brief Represents a memory allocator with function pointers for memory management.
 */
typedef struct
{
	void *context;												  /**< The context associated with the allocator. */
	void *(*allocate)(void *context, cff_size size);			  /**< Function pointer for memory allocation. */
	void (*release)(void *context, void *ptr);					  /**< Function pointer for releasing allocated memory. */
	void *(*reallocate)(void *context, void *ptr, cff_size size); /**< Function pointer for reallocating memory. */
	cff_size (*get_size)(void *context, void *ptr);				  /**< Function pointer for retrieving the size of allocated memory. */
} cff_allocator_t;

/**
 * @brief Allocates memory using the specified allocator.
 *
 * @param alloc The allocator to use for memory allocation.
 * @param size The size of the memory to allocate.
 * @return A pointer to the allocated memory.
 */
inline void *cff_allocator_allocate(cff_allocator_t *alloc, cff_size size) { return alloc->allocate(alloc->context, size); }

/**
 * @brief Releases memory using the specified allocator.
 *
 * @param alloc The allocator to use for memory release.
 * @param ptr The pointer to the memory to release.
 */
inline void cff_allocator_release(cff_allocator_t *alloc, void *ptr) { alloc->release(alloc->context, ptr); }

/**
 * @brief Reallocates memory using the specified allocator.
 *
 * @param alloc The allocator to use for memory reallocation.
 * @param ptr The pointer to the memory to reallocate.
 * @param size The new size of the memory.
 * @return A pointer to the reallocated memory.
 */
inline void *cff_allocator_reallocate(cff_allocator_t *alloc, void *ptr, cff_size size) { return alloc->reallocate(alloc->context, ptr, size); }

/**
 * @brief Retrieves the size of allocated memory using the specified allocator.
 *
 * @param alloc The allocator to use for retrieving the size.
 * @param ptr The pointer to the allocated memory.
 * @return The size of the allocated memory.
 */
inline cff_size cff_allocator_get_size(cff_allocator_t *alloc, void *ptr) { return alloc->get_size(alloc->context, ptr); }

/**
 * @brief Allocates memory using the default allocator.
 *
 * @param size The size of the memory to allocate.
 * @return A pointer to the allocated memory.
 */
void *cff_malloc(cff_size size);

/**
 * @brief Allocates memory from the stack.
 *
 * @param size The size of the memory to allocate.
 * @return A pointer to the allocated memory.
 */
void *cff_stack_alloc(cff_size size);

/**
 * @brief Reallocates memory.
 *
 * @param ptr The pointer to the memory to reallocate.
 * @param size The new size of the memory.
 * @return A pointer to the reallocated memory.
 */
void *cff_realloc(void *ptr, cff_size size);

/**
 * @brief Frees memory allocated with cff_malloc or cff_realloc.
 *
 * @param ptr The pointer to the memory to free.
 */
void cff_free(void *ptr);

/**
 * @brief Frees memory allocated from the stack.
 *
 * @param ptr The pointer to the memory to free.
 */
void cff_stack_free(void *ptr);

/**
 * @brief Retrieves the size of allocated memory.
 *
 * @param ptr The pointer to the allocated memory.
 * @return The size of the allocated memory.
 */
cff_size cff_get_size(void *ptr);

/**
 * @brief Copies memory from one location to another.
 *
 * @param from The source memory location.
 * @param dest The destination memory location.
 * @param size The number of bytes to copy.
 */
void cff_mem_copy(const void *from, void *dest, cff_size size);

/**
 * @brief Moves memory from one location to another, handling overlapping memory blocks.
 *
 * @param from The source memory location.
 * @param dest The destination memory location.
 * @param size The number of bytes to move.
 */
void cff_mem_move(const void *from, void *dest, cff_size size);

/**
 * @brief Compares memory blocks for equality.
 *
 * @param from The first memory block to compare.
 * @param dest The second memory block to compare.
 * @param size The number of bytes to compare.
 * @return True if the memory blocks are equal, false otherwise.
 */
bool cff_mem_cmp(const void *const from, const void *const dest, cff_size size);

/**
 * @brief Sets a block of memory to a specific value.
 *
 * @param data The data to set the memory to.
 * @param dest The memory block to set.
 * @param data_size The size of the data.
 * @param buffer_length The length of the memory block.
 */
void cff_mem_set(const void *data, void *dest, cff_size data_size, cff_size buffer_lenght);