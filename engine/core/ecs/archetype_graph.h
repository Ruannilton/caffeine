#pragma once

#include "archetype.h"

bool graph_init(uint32_t archetype_count);
void graph_release();
entity_id graph_alloc_entity(archetype_id arch_id);
bool graph_add_archetype(archetype_id id, archetype *archetype);
void *graph_get_entity_component(entity_id ent_id, component_id comp_id);