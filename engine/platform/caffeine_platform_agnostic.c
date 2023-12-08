#include "caffeine_platform.h"
#include <malloc.h>
#include <stdlib.h>

static inline void internal_cff_mem_copy(const void *const from_ref, void *const dest_mut_ref,
                                         uint64_t size)
{
  const uintptr_t *f = (const uintptr_t *)(from_ref);
  uintptr_t *d = (uintptr_t *)(dest_mut_ref);

  while (size >= sizeof(uintptr_t))
  {
    *(d++) = *(f++);
    size -= sizeof(uintptr_t);
  }

  const uint8_t *char_f = (const uint8_t *)f;
  uint8_t *char_d = (uint8_t *)d;

  while (size)
  {
    *(char_d++) = *(char_f++);
    size -= sizeof(uint8_t);
  }
}

void *cff_malloc(uint64_t size) { return malloc((size_t)size); }

void *cff_stack_alloc(uint64_t size)
{
#ifdef CFF_MSVC
  return _malloca((size_t)size);
#elif CFF_GCC
  return alloca(size);
#endif
}

void *cff_realloc(const void *ptr_owning, uint64_t size)
{
  assert(ptr_owning != NULL);
  return realloc((void *)ptr_owning, (size_t)size);
}

void cff_free(const void *const ptr_owning)
{
  assert(ptr_owning != NULL);
  free((void *)ptr_owning);
}

void cff_stack_free(void *ptr)
{
  assert(ptr != NULL);

#ifdef CFF_MSVC
  _freea(ptr);
#elif CFF_GCC

#endif
}

void cff_mem_copy(const void *const from_ref, void *const dest_mut_ref, uint64_t size)
{

  if (from_ref == dest_mut_ref || size == 0)
    return;

  internal_cff_mem_copy(from_ref, dest_mut_ref, size);
}

void cff_mem_move(const void *const from_ref, void *const dest_mut_ref, uint64_t size)
{
  const uint8_t *f = (const uint8_t *)from_ref;
  uint8_t *d = (uint8_t *)dest_mut_ref;

  if (f == d || size == 0)
    return;

  if (d > f && ((unsigned int)(d - f)) < size)
  {
    int i;
    for (i = (int)size - 1; i >= 0; i--)
      d[i] = f[i];
    return;
  }

  if (f > d && ((unsigned int)(f - d)) < size)
  {
    size_t i;
    for (i = 0; i < size; i++)
      d[i] = f[i];
    return;
  }

  cff_mem_copy(from_ref, dest_mut_ref, size);
}

void cff_mem_set(const void *const data_ref, void *const dest_mut_ref, uint64_t data_size,
                 uint64_t buffer_lenght)
{
  assert(buffer_lenght % data_size == 0);

  uintptr_t dest_start = (uintptr_t)dest_mut_ref;

  for (uint64_t i = 0; i < buffer_lenght; i += data_size)
  {
    void *address = (void *)(dest_start + i);
    internal_cff_mem_copy(data_ref, address, data_size);
  }
}

void cff_mem_zero(void *const dest_mut_ref, uint64_t buffer_lenght)
{
  char *const buffer = (char *const)dest_mut_ref;
  for (size_t i = 0; i < buffer_lenght; i++)
  {
    buffer[i] = 0;
  }
}

bool cff_mem_cmp(const void *const from_ref, const void *const dest_mut_ref, uint64_t size)
{
  const uintptr_t *f = (const uintptr_t *)(from_ref);
  const uintptr_t *d = (const uintptr_t *)(dest_mut_ref);

  if (from_ref == dest_mut_ref || size == 0)
    return true;

  while (size >= sizeof(uintptr_t))
  {
    if (*(d++) != *(f++))
      return false;
    size -= sizeof(uintptr_t);
  }

  const uint8_t *char_f = (const uint8_t *)f;
  const uint8_t *char_d = (const uint8_t *)d;

  while (size)
  {
    if (*(char_d++) != *(char_f++))
      return false;
    size -= sizeof(uint8_t);
  }

  return true;
}

// TODO: implement
uint64_t cff_get_size(void *ptr)
{
  (void)ptr;
  return 0;
}

void default_key_clkb(uint32_t key, uint32_t state)
{
  (void)key;
  (void)state;
}
void default_mouse_button_clkb(uint32_t button, uint32_t state)
{
  (void)button;
  (void)state;
}
void default_mouse_move_clkb(uint32_t x, uint32_t y)
{
  (void)x;
  (void)y;
}
void default_mouse_scroll_clkb(int32_t dir) { (void)dir; }
void default_quit(void) {}
void default_resize(uint32_t width, uint32_t lenght)
{
  (void)width;
  (void)lenght;
}
