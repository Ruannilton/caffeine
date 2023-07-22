#pragma once

#include "caffeine_container_type.h"


struct caffeine_ord_container_s {
  uint64_t capacity;
  cff_size data_size;
  cff_err_e error_code;
  uintptr_t buffer;
  cff_order_function comparer;
};

typedef struct caffeine_ord_container_s cff_ord_container_s;