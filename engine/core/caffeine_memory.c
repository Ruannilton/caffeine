#include "caffeine_memory.h"
#include "../platform/caffeine_platform.h"

#ifdef CFF_DEBUG
#include <stdio.h>
static uint64_t _mem_allocked;

typedef struct
{
  uint32_t size;
} mem_header;

#endif

void cff_memory_init()
{
#ifdef CFF_DEBUG
  _mem_allocked = 0;
#endif
}

void cff_memory_end()
{
#ifdef CFF_DEBUG
  char msg[64];
  sprintf(msg, "Bytes not freed: %llu\n", _mem_allocked);
  cff_print_console(LOG_LEVEL_INFO, msg);
#endif
}

void *cff_mem_alloc(uint64_t size)
{
#ifdef CFF_DEBUG
  uintptr_t ptr = (uintptr_t)cff_malloc(sizeof(mem_header) + size);
  if (ptr != 0)
  {
    mem_header *header = (mem_header *)ptr;
    header->size = size;
    _mem_allocked += size;

    ptr += sizeof(mem_header);
  }
  return (void *)ptr;
#else
  void *ptr = cff_malloc(size);
  return ptr;
#endif
}

void *cff_mem_realloc(void *ptr, uint64_t size)
{
#ifdef CFF_DEBUG
  if (ptr != NULL)
  {
    mem_header *header = (mem_header *)((uintptr_t)ptr - sizeof(mem_header));
    _mem_allocked -= header->size;

    mem_header *nheader = cff_realloc(header, size);
    nheader->size = size;
    _mem_allocked += size;

    void *nptr = (void *)(((uintptr_t)nheader) + sizeof(mem_header));
    return nptr;
  }
#else
  if (ptr != NULL)
    return cff_realloc(ptr, size);
#endif
  return NULL;
}

void cff_mem_release(void *ptr)
{
#ifdef CFF_DEBUG
  if (ptr != NULL)
  {
    mem_header *header = (mem_header *)((uintptr_t)ptr - sizeof(mem_header));
    _mem_allocked -= header->size;
    cff_free(header);
  }
#else
  if (ptr != NULL)
    cff_free(ptr);
#endif
}
