#pragma once

#include "ecs_types.h"

typedef struct archetype_index archetype_index;

archetype_index *ecs_new_archetype_index(uint32_t capacity);

archetype_id ecs_register_archetype(archetype_index *index, uint32_t count, component_id components[]);

archetype_id ecs_get_archetype_id(archetype_index *index, uint32_t count, component_id components[]);

bool ecs_archetype_has_component(archetype_index *index, archetype_id archetype, component_id component);

void ecs_archetype_add_component(archetype_index *index, archetype_id archetype, component_id component);

void ecs_archetype_remove_component(archetype_index *index, archetype_id archetype, component_id component);

void ecs_remove_archetype(archetype_index *index, archetype_id id);

void ecs_release_archetype_index(archetype_index *index);

uint32_t ecs_archetype_get_components(archetype_index *index, archetype_id id, const component_id *out);