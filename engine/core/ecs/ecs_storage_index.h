#pragma once

#include "ecs_types.h"

typedef struct storage_index storage_index;
typedef struct ecs_storage ecs_storage;

storage_index *ecs_storage_index_new(uint32_t capacity);
void ecs_storage_index_release(storage_index *index);

void ecs_storage_index_set(storage_index *index, archetype_id arch_id, ecs_storage *storage);
ecs_storage *ecs_storage_index_get(storage_index *index, archetype_id arch_id);
void ecs_storage_index_remove(storage_index *index, archetype_id arch_id);