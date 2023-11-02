#pragma once

#include "ecs_types.h"

typedef struct ecs_world ecs_world;

ecs_world *ecs_world_new();
void ecs_world_release(ecs_world *world);

component_id ecs_world_add_component(ecs_world *world, const char *name, size_t size, size_t align);
void ecs_world_remove_component(ecs_world *world, component_id id);

archetype_id ecs_world_add_archetype(ecs_world *world, ecs_archetype archetype);
void ecs_world_remove_archetype(ecs_world *world, archetype_id id);

entity_id ecs_world_create_entity(ecs_world *world, archetype_id id);
void ecs_world_destroy_entity(ecs_world *world, entity_id id);
void *ecs_world_get_entity_component(ecs_world *world, entity_id entity, component_id component);
void ecs_world_set_entity_component(ecs_world *world, entity_id entity, component_id component, void *data);

void ecs_worl_register_system(ecs_world *world, ecs_query *query, ecs_system system);