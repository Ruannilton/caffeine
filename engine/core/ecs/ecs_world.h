#pragma once

#include "ecs_types.h"

typedef struct ecs_world ecs_world;

ecs_world *ecs_world_new();
void ecs_world_release(const ecs_world *const world_owning);

component_id ecs_world_add_component(const ecs_world *const world_ref, const char *name, size_t size, size_t align);
void ecs_world_remove_component(const ecs_world *const world_ref, component_id id);

archetype_id ecs_world_add_archetype(const ecs_world *const world_ref, ecs_archetype archetype);
void ecs_world_remove_archetype(const ecs_world *const world_ref, archetype_id id);

entity_id ecs_world_create_entity(const ecs_world *const world_ref, archetype_id id);
void ecs_world_destroy_entity(const ecs_world *const world_ref, entity_id id);
void *ecs_world_get_entity_component(const ecs_world *const world_ref, entity_id entity, component_id component);
void ecs_world_set_entity_component(const ecs_world *const world_ref, entity_id entity, component_id component, void *data);

void ecs_worl_register_system(const ecs_world *const world_ref, ecs_query *query, ecs_system system);