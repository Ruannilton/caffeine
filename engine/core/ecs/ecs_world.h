#pragma once

#include "../../caffeine_types.h"
#include "ecs_types.h"
#include "ecs_query.h"

typedef struct ecs_world ecs_world;

ecs_world *ecs_world_new();
void ecs_world_release(const ecs_world *const world_owning);

CAFF_API component_id ecs_world_add_component(const ecs_world *const world_ref, const char *name, size_t size, size_t align);
CAFF_API void ecs_world_remove_component(const ecs_world *const world_ref, component_id id);

CAFF_API archetype_id ecs_world_add_archetype(const ecs_world *const world_ref, ecs_archetype archetype);
CAFF_API void ecs_world_remove_archetype(const ecs_world *const world_ref, archetype_id id);

CAFF_API entity_id ecs_world_create_entity(const ecs_world *const world_ref, archetype_id id);
CAFF_API void ecs_world_destroy_entity(const ecs_world *const world_ref, entity_id id);
CAFF_API void *ecs_world_get_entity_component(const ecs_world *const world_ref, entity_id entity, component_id component);
CAFF_API void ecs_world_set_entity_component(const ecs_world *const world_ref, entity_id entity, component_id component, void *data);

CAFF_API void ecs_worl_register_system(const ecs_world *const world_ref, ecs_query *query, ecs_system system);

void ecs_world_step(const ecs_world *const world_ref);