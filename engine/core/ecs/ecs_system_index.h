#pragma once

#include "ecs_types.h"

typedef struct system_index system_index;
typedef struct storage_index storage_index;

system_index ecs_system_index_new(uint32_t capacity, storage_index *storage);

void ecs_system_index_add(system_index *index, ecs_query *query, uint32_t storage_count, ecs_system system);
void ecs_system_index_add_archetype(system_index *index);