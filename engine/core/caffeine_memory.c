#include "caffeine_memory.h"
#include "../platform/caffeine_platform.h"
#include "allocators/cff_allocator.h"
#include "allocators/system_allocator.h"

static struct cff_allocator_t _global_allocator;

static cff_allocator global_allocator = &_global_allocator;

#ifdef CFF_DEBUG
#include <stdio.h>
static uint64_t _mem_allocked;
#endif

cff_allocator cff_memory_get_global()
{
  return &_global_allocator;
}

void cff_memory_init()
{
#ifdef CFF_DEBUG
  _mem_allocked = 0;
#endif
  _global_allocator = cff_allocator_create_system();
}

void cff_memory_end()
{
#ifdef CFF_DEBUG
  char msg[64];
  sprintf(msg, "Bytes not freed: %llu\n", _mem_allocked);
  cff_print_console(LOG_LEVEL_INFO, msg);
#endif
}

cff_err_e cff_memory_alloc(cff_size size, uintptr_t *out)
{
  return cff_allocator_alloc(global_allocator, size, out);
}

cff_err_e cff_memory_realloc(uintptr_t ptr, cff_size size, uintptr_t *out)
{
  return cff_allocator_realloc(global_allocator, ptr, size, out);
}

cff_err_e cff_memory_release(uintptr_t ptr)
{
  return cff_allocator_release(global_allocator, ptr);
}

cff_err_e cff_memory_get_size(uintptr_t ptr, cff_size *size)
{
  return cff_allocator_get_size(global_allocator, ptr, size);
}

cff_err_e cff_allocator_alloc(cff_allocator alloc, cff_size size,
                              uintptr_t *out)
{

  cff_err_e err = alloc->allocate(alloc->context, size, out);
#ifdef CFF_DEBUG
  if (!IS_ERROR(err))
    _mem_allocked += size;
#endif

  return err;
}

cff_err_e cff_allocator_realloc(cff_allocator alloc, uintptr_t ptr,
                                cff_size size, uintptr_t *out)
{

#ifdef CFF_DEBUG
  cff_size ss = 0;
  cff_memory_get_size(ptr, &ss);
#endif

  cff_err_e err = alloc->reallocate(alloc->context, ptr, size, out);

#ifdef CFF_DEBUG
  if (!IS_ERROR(err))
  {
    _mem_allocked -= ss;
    _mem_allocked += size;
  }
#endif

  return err;
}

cff_err_e cff_allocator_release(cff_allocator alloc, uintptr_t ptr)
{

#ifdef CFF_DEBUG
  cff_size ss = 0;
  cff_memory_get_size(ptr, &ss);
#endif

  cff_err_e err = alloc->release(alloc->context, ptr);

#ifdef CFF_DEBUG
  if (!IS_ERROR(err))
  {
    _mem_allocked -= ss;
  }
#endif

  return err;
}

cff_err_e cff_allocator_get_size(cff_allocator alloc, uintptr_t ptr,
                                 cff_size *size)
{
  return alloc->get_size(alloc->context, ptr, size);
  return CFF_ERR_NONE;
}
