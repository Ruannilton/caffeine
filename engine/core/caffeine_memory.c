#include "caffeine_memory.h"
#include "caffeine_platform.h"

static void *allocate(void *context, cff_size size) { return cff_malloc(size); }

static void release(void *context, void *ptr) { cff_free(ptr); }

static void *reallocate(void *context, void *ptr, cff_size size) {
  return cff_realloc(ptr, size);
}

static cff_size get_size(void *context, void *ptr) { return cff_get_size(ptr); }

cff_allocator_t cff_global_allocator = {.context = 0,
                                        .allocate = allocate,
                                        .get_size = get_size,
                                        .reallocate = reallocate,
                                        .release = release};