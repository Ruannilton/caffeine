#pragma once

#include "ecs_types.h"

typedef struct archetype_graph archetype_graph;

archetype_graph *ecs_archetype_graph_new();
void ecs_archetype_graph_release(const archetype_graph *const graph_ref);
void ecs_archetype_graph_add(archetype_graph *graph, archetype_id id, uint32_t count, const component_id components[]);
uint32_t ecs_archetype_graph_find_with(const archetype_graph *const graph_ref, uint32_t count, const component_id components[], archetype_id **const out_mut_ref);