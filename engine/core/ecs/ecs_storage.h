#pragma once

#include "ecs_types.h"

typedef struct ecs_storage ecs_storage;

int ecs_storage_add_entity(ecs_storage *const storage, entity_id entity);
entity_id ecs_storage_remove_entity(ecs_storage *const storage, int row);

void ecs_storage_set_component(ecs_storage *const storage_mut_ref, int row, component_id component, const void *const data);
void *ecs_storage_get_component(const ecs_storage *const storage_ref, int row, component_id component);
component_id ecs_storage_get_component_id(const ecs_storage *const storage_ref, const char *const name);

void *ecs_storage_get_component_list(const ecs_storage *const storage_ref, component_id component);
entity_id *ecs_storage_get_enetities_ids(const ecs_storage *const storage_ref);

int ecs_storage_move_entity(ecs_storage *const from_storage_ref, ecs_storage *const to_storage_mut_ref, entity_id id, int entity_row);

uint32_t ecs_storage_count(const ecs_storage *const storage_ref);