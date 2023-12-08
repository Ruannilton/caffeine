#pragma once

#include "ecs_types.h"

typedef struct storage_index storage_index;
typedef struct ecs_storage ecs_storage;

storage_index *ecs_storage_index_new(uint32_t capacity);
void ecs_storage_index_release(const storage_index *const index);

void ecs_storage_index_new_storage(storage_index *const index, archetype_id arch_id, component_id *components, size_t *sizes, uint32_t lenght);
ecs_storage *ecs_storage_index_get(const storage_index *const index, archetype_id arch_id);
void ecs_storage_index_remove(storage_index *const index, archetype_id arch_id);