#include "system_allocator.h"
#include "../../platform/caffeine_platform.h"
#include "cff_allocator.h"

cff_err_e allocate(void *context, cff_size size, uintptr_t *out) {

  cff_size total_size = size + HEADER_SIZE;

  struct cff_block_t *block = cff_malloc(total_size);

  if (block == NULL)
    return CFF_ERR_ALLOC;

  SET_SIZE(block, size);

  uintptr_t data = (uintptr_t)SKIP_HEADER(block);

  *out = data;

  return CFF_ERR_NONE;
}

cff_err_e release(void *context, uintptr_t ptr) {
  struct cff_block_t *block = GET_HEADER(ptr);

  cff_free((void *)block);
  return CFF_ERR_NONE;
}

cff_err_e reallocate(void *context, uintptr_t ptr, cff_size size,
                     uintptr_t *out) {

  struct cff_block_t *block = GET_HEADER(ptr);

  cff_size total_size = size + HEADER_SIZE;

  struct cff_block_t *new_block =
      (struct cff_block_t *)cff_realloc((void *)block, total_size);

  if (new_block == NULL)
    return CFF_ERR_ALLOC;

  SET_SIZE(new_block, size);
  uintptr_t data = (uintptr_t)SKIP_HEADER(new_block);

  *out = data;

  return CFF_ERR_NONE;
}

cff_err_e get_size(void *context, uintptr_t ptr, cff_size *out) {

  struct cff_block_t *block = GET_HEADER(ptr);
  uint64_t size = GET_SIZE(block);

  if (size == (uint64_t)-1) {
    return CFF_ERR_INVALID_OPERATION;
  }

  *out = (cff_size)size;
  return CFF_ERR_NONE;
}

struct cff_allocator_t cff_allocator_create_system() {

  struct cff_allocator_t _system_allocaor = (struct cff_allocator_t){
      .context = NULL,
      .allocate = allocate,
      .reallocate = reallocate,
      .release = release,
      .get_size = get_size,
  };

  return _system_allocaor;
}