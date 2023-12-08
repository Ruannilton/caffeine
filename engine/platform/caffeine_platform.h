#pragma once

#include "../caffeine_types.h"

typedef void (*cff_platform_key_clkb)(uint32_t key, uint32_t state);
typedef void (*cff_platform_mouse_button_clkb)(uint32_t button, uint32_t state);
typedef void (*cff_platform_mouse_move_clkb)(uint32_t x, uint32_t y);
typedef void (*cff_platform_mouse_scroll_clkb)(int32_t dir);
typedef void (*cff_platform_quit_clbk)(void);
typedef void (*cff_platform_resize_clbk)(uint32_t width, uint32_t lenght);

bool cff_platform_init(char *name);

bool cff_platform_poll_events();

void cff_platform_shutdown();

void cff_platform_set_key_clbk(cff_platform_key_clkb clbk);

void cff_platform_set_mouse_button_clkb(cff_platform_mouse_button_clkb clbk);

void cff_platform_set_mouse_move_clkb(cff_platform_mouse_move_clkb clbk);

void cff_platform_set_mouse_scroll_clkb(cff_platform_mouse_scroll_clkb clbk);

void cff_platform_set_quit_clkb(cff_platform_quit_clbk clbk);

void cff_platform_set_resize_clkb(cff_platform_resize_clbk clbk);

/**
 * @brief Allocates memory using the default allocator.
 *
 * @param size The size of the memory to allocate.
 * @return A pointer to the allocated memory.
 */
void *cff_malloc(uint64_t size);

/**
 * @brief Allocates memory from the stack.
 *
 * @param size The size of the memory to allocate.
 * @return A pointer to the allocated memory.
 */
void *cff_stack_alloc(uint64_t size);

/**
 * @brief Reallocates memory.
 *
 * @param ptr The pointer to the memory to reallocate.
 * @param size The new size of the memory.
 * @return A pointer to the reallocated memory.
 */
void *cff_realloc(const void *ptr_owning, uint64_t size);

/**
 * @brief Frees memory allocated with cff_malloc or cff_realloc.
 *
 * @param ptr The pointer to the memory to free.
 */
void cff_free(const void *const ptr_owning);

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
uint64_t cff_get_size(void *ptr);

/**
 * @brief Copies memory from one location to another.
 *
 * @param from The source memory location.
 * @param dest The destination memory location.
 * @param size The number of bytes to copy.
 */
void cff_mem_copy(const void *from, void *dest, uint64_t size);

/**
 * @brief Moves memory from one location to another, handling overlapping memory
 * blocks.
 *
 * @param from The source memory location.
 * @param dest The destination memory location.
 * @param size The number of bytes to move.
 */
void cff_mem_move(const void *from, void *dest, uint64_t size);

/**
 * @brief Compares memory blocks for equality.
 *
 * @param from The first memory block to compare.
 * @param dest The second memory block to compare.
 * @param size The number of bytes to compare.
 * @return True if the memory blocks are equal, false otherwise.
 */
bool cff_mem_cmp(const void *const from, const void *const dest, uint64_t size);

/**
 * @brief Sets a block of memory to a specific value.
 *
 * @param data The data to set the memory to.
 * @param dest The memory block to set.
 * @param data_size The size of the data.
 * @param buffer_length The length of the memory block.
 */
void cff_mem_set(const void *data, void *dest, uint64_t data_size,
                 uint64_t buffer_lenght);

void cff_mem_zero(void *const dest, uint64_t buffer_lenght);

void cff_print_console(log_level level, const char *const message);

void cff_print_error(log_level level, const char *const message);

void *cff_platform_create_file(const char *path);

void *cff_platform_open_file(const char *path, file_attributes attributes);

cff_err_e cff_platform_file_write(void *file, void *data, uint64_t data_size);

cff_err_e cff_platform_file_close(void *file);

cff_err_e cff_platform_file_delete(const char *path);

bool cff_platform_file_exists(const char *path);

uint64_t cff_platform_file_size(void *file);

const char *cff_get_app_directory();

const char *cff_get_app_data_directory();