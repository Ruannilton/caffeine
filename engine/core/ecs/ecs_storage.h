#pragma once

#include "ecs_types.h"

typedef struct ecs_storage ecs_storage;

int ecs_storage_add_entity(ecs_storage *const storage, entity_id entity);
entity_id ecs_storage_remove_entity(ecs_storage *const storage, int row);

void ecs_storage_set_component(ecs_storage *const storage, int row, component_id component, void *data);
void *ecs_storage_get_component(ecs_storage *const storage, int row, component_id component);