#include "caffeine_memory.h"
#include "../platform/caffeine_platform.h"

#ifdef CFF_DEBUG
#include <stdio.h>
static uint64_t _mem_allocked;
static uint64_t _count = 0;

typedef struct
{
  uint32_t id;
  uint32_t size;
  uint8_t freed;
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
  sprintf_s(msg, 64, "Bytes not freed: %llu\n", _mem_allocked);
  cff_print_console(LOG_LEVEL_INFO, msg);
#endif
}

void *cff_mem_alloc(uint64_t size)
{
#ifdef CFF_DEBUG
  uintptr_t ptr = (uintptr_t)cff_malloc(sizeof(mem_header) + size);
  if (ptr != 0)
  {
    _count++;
    mem_header *header = (mem_header *)ptr;
    header->size = size;
    header->id = _count;
    header->freed = 0;
    _mem_allocked += size;

    char msg[128];
    sprintf_s(msg, 128, "%llu - [%p] Allocated: %llu | Total: %llu\n", _count, (void *)ptr, size, _mem_allocked);
    cff_print_console(LOG_LEVEL_INFO, msg);

    ptr += sizeof(mem_header);
  }
  return (void *)ptr;
#else
  void *ptr = cff_malloc(size);
  return ptr;
#endif
}

void *cff_mem_realloc(const void *ptr_owning, uint64_t size)
{
#ifdef CFF_DEBUG
  if (ptr_owning != NULL)
  {
    mem_header *header = (mem_header *)((uintptr_t)ptr_owning - sizeof(mem_header));
    _mem_allocked -= header->size;

    mem_header *nheader = (mem_header *)cff_realloc(header, size + sizeof(mem_header));
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

void cff_mem_release(const void *const ptr_owning)
{
#ifdef CFF_DEBUG
  if (ptr_owning != NULL)
  {
    mem_header *header = (mem_header *)((uintptr_t)ptr_owning - sizeof(mem_header));
    char msg[256] = {0};

    if (header->freed == 1)
    {
      cff_print_console(LOG_LEVEL_INFO, (const char *const)"Double Free Detected!\n");
      return;
    }

    header->freed = 1;
    _mem_allocked -= header->size;

    sprintf_s(msg, 256, "%u - [%p | %p] Freeded: %u | Tota: %llu\n", header->id, (void *)header, ptr_owning, header->size, _mem_allocked);
    cff_print_console(LOG_LEVEL_INFO, msg);
    cff_free(header);
  }
#else
  if (ptr != NULL)
    cff_free(ptr);
#endif
}
