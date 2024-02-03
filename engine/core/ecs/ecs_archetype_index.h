#pragma once

#include "ecs_types.h"

/*
    estrutura para armazenar os archetypes, internamente utiliza um hashmap para relacionar o id ao dados do archetype.

*/
typedef struct archetype_index archetype_index;

archetype_index *ecs_new_archetype_index(uint32_t capacity);

archetype_id ecs_register_archetype(archetype_index *index, ecs_archetype archetype_owning);

archetype_id ecs_get_archetype_id(const archetype_index *const index_ref, uint32_t count, const component_id *const components);

bool ecs_archetype_has_component(const archetype_index *const index, archetype_id archetype, component_id component);

void ecs_remove_archetype(archetype_index *index, archetype_id id);

void ecs_release_archetype_index(const archetype_index *const index);

uint32_t ecs_archetype_get_components(const archetype_index *const index_ref, archetype_id id, const component_id **out_mut_ref);

archetype_id ecs_archetype_add_component(archetype_index *const index_mut_ref, archetype_id origin_arch_id, component_id component);

archetype_id ecs_archetype_remove_component(archetype_index *const index_mut_ref, archetype_id origin_arch_id, component_id component);