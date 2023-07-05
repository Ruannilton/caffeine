#pragma once

#include <caffeine_core.h>
#include <caffeine_memory.h>
#include "caffeine_container_type.h"

typedef struct {
	cff_size(*get_data_size)(cff_container_s*);
	uint64_t(*get_capacity)(cff_container_s*);
	uint64_t(*get_count)(cff_container_s*);
	cff_err_e(*get_item)(cff_container_s*, uint64_t, uintptr_t);
	cff_err_e(*get_item_ref)(cff_container_s*, uint64_t, uintptr_t*);
	cff_err_e(*add_item)(cff_container_s*, uint64_t, uintptr_t, cff_allocator_t);
	cff_err_e(*remove_item)(cff_container_s*, uint64_t, cff_allocator_t);
}cff_container_i;