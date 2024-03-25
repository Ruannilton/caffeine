#pragma once

#include "ecs_component_index.h"

#include "../ds/caffeine_hashmap.h"

typedef const char *caff_string;

cff_hash_dcltype(name_index, caff_string, uint64_t);

void ecs_name_index_init(name_index *index);
uint8_t ecs_name_index_get(const name_index *index, caff_string name, uint64_t *out);
bool ecs_name_index_remove(name_index *index, caff_string name);
void ecs_name_index_add(name_index *index, caff_string name, uint64_t id);
void ecs_name_index_release(const name_index *index);