#include "caffeine_memory.h"
#include "../platform/caffeine_platform.h"

#ifdef CFF_DEBUG
#include <stdio.h>
static uint64_t _mem_allocked;
static uint64_t _count = 0;

typedef struct
{
  const char *block_name;
  const char *file;
  uint64_t line;
  uint32_t id;
  uint32_t size;
  uint8_t freed;
} mem_header;

#endif

static int _get_id()
{
  _count++;
  return _count;
}

static mem_header *_get_header(const void *ptr)
{
  mem_header *result = (mem_header *)(((uintptr_t)ptr) - sizeof(mem_header));
  return result;
}

static void *_get_block(mem_header *header)
{
  void *result = (void *)(((uintptr_t)header) + sizeof(mem_header));
  return result;
}

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
  void *ptr = cff_malloc(size);
  return ptr;
}

void *cff_mem_realloc(const void *ptr_owning, uint64_t size)
{
  if (ptr_owning != NULL)
    return cff_realloc(ptr_owning, size);
  return NULL;
}

void cff_mem_release(const void *const ptr_owning)
{
  if (ptr_owning != NULL)
    cff_free(ptr_owning);
}

#ifdef CFF_DEBUG

void *cff_mem_alloc_dbg(uint64_t size, const char *const block_name, const char *const file, uint64_t line)
{
  uint64_t debug_size = size + sizeof(mem_header);

  mem_header *debug_result = (mem_header *)cff_mem_alloc(debug_size);

  debug_result->block_name = block_name;
  debug_result->file = file;
  debug_result->line = line;
  debug_result->id = _get_id();
  debug_result->size = size;
  debug_result->freed = 0;

  void *result = _get_block(debug_result);

  {
    char msg[128];
    sprintf_s(msg, 128, "%llu - [%p] Allocated: %llu | Total: %llu\n", _count, (void *)debug_result, size, _mem_allocked);
  }

  return result;
}

void *cff_mem_realloc_dbg(const void *ptr_owning, uint64_t size, const char *const file, uint64_t line)
{

  mem_header *old_header = _get_header(ptr_owning);
  _mem_allocked -= old_header->size;

  mem_header *new_header = (mem_header *)cff_mem_realloc(old_header, size + sizeof(mem_header));
  new_header->size = size;
  _mem_allocked -= size;

  void *result = _get_block(new_header);
  return result;
}

void cff_mem_release_dbg(const void *const ptr_owning, const char *const file, uint64_t line)
{
  mem_header *header = _get_header(ptr_owning);

  if (header->freed)
  {
    cff_print_console(LOG_LEVEL_INFO, (const char *const)"Double Free Detected!\n");
    return;
  }

  header->freed = 1;
  _mem_allocked -= header->size;

  cff_mem_release(header);

  {
    char msg[256] = {0};
    sprintf_s(msg, 256, "%u - [%p | %p] Freeded: %u | Tota: %llu\n", header->id, (void *)header, ptr_owning, header->size, _mem_allocked);
    cff_print_console(LOG_LEVEL_INFO, msg);
  }
}

void cff_mem_copy_dbg(const void *const from_ref, void *const dest_mut_ref, uint64_t size, const char *const file, uint64_t line)
{
  cff_mem_copy(from_ref, dest_mut_ref, size);
}

void cff_mem_move_dbg(const void *const from_ref, void *const dest_mut_ref, uint64_t size, const char *const file, uint64_t line)
{
  cff_mem_move(from_ref, dest_mut_ref, size);
}

bool cff_mem_cmp_dbg(const void *const from_ref, const void *const dest_ref, uint64_t size, const char *const file, uint64_t line)
{
  bool result = cff_mem_cmp(from_ref, dest_ref, size);
  return result;
}

void cff_mem_set_dbg(const void *const data_ref, void *const dest_mut_ref, uint64_t data_size, uint64_t buffer_lenght, const char *const file, uint64_t line)
{
  cff_mem_set(data_ref, dest_mut_ref, data_size, buffer_lenght);
}

void cff_mem_zero_dbg(void *const dest_mut_ref, uint64_t buffer_lenght, const char *const file, uint64_t line)
{
  cff_mem_zero(dest_mut_ref, buffer_lenght);
}

#endif