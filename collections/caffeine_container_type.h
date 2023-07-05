#pragma once

#include <caffeine_types.h>

struct caffeine_container_s {
  uint64_t capacity;
  cff_size data_size;
  cff_err_e error_code;
  uintptr_t buffer;
};

typedef struct caffeine_container_s cff_container_s;