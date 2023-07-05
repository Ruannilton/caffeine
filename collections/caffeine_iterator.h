#pragma once

#include "caffeine_container_type.h"
#include <caffeine_memory.h>
#include <caffeine_types.h>

typedef struct {
  uint64_t (*get_capacity)(void *);
  uint64_t (*get_count)(void *);
  bool (*get_item_ref)(void *, uint64_t, uintptr_t *);
  cff_size (*get_data_size)(void *);
} cff_container_iterator_i;

typedef struct {
  uint64_t index;
  uintptr_t current_item;
  void *data;
  cff_container_iterator_i interf;
} cff_iterator_s;

uint64_t cff_iterator_count(cff_iterator_s *it);

cff_size cff_iterador_data_size(cff_iterator_s *it);

uint64_t cff_iterator_index(cff_iterator_s *it);

void cff_iterator_reset(cff_iterator_s *it);

bool cff_iterator_next(cff_iterator_s *it);

uintptr_t cff_iterator_current(cff_iterator_s *it);