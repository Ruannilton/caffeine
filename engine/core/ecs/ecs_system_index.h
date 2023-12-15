#pragma once

#include "ecs_types.h"

typedef struct system_index system_index;
typedef struct storage_index storage_index;

system_index *ecs_system_index_new(const storage_index *const storage_index, uint32_t capacity);
void ecs_system_index_release(system_index *index);

void ecs_system_index_add(system_index *index, ecs_query *query, archetype_id *archetypes, uint32_t archetypes_count, ecs_system system);
void ecs_system_index_add_archetype(system_index *index, archetype_id archetype, const component_id *components, uint32_t component_count);
void ecs_system_step(system_index *index);