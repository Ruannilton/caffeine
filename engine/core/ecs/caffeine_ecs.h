#pragma once

#include "../ds/caffeine_ds.h"
#include <stdint.h>

#define INVALID_ID (0xffffffff)

typedef uint32_t component_id;
typedef uint32_t archtype_id;
typedef uint32_t entity_id;
typedef uint32_t query_id;

cff_arr_dcltype(archetype, component_id);

typedef struct query_it query_it;

typedef struct
{
    archetype with_components;
    bool exact;
} ecs_query;

typedef void (*ecs_system)(query_it *iterator);

bool ecs_init();
bool ecs_end();

component_id ecs_register_component(const char *name, uint32_t size);
archtype_id ecs_register_archetype(archetype arch);
archtype_id ecs_archetype_get(uint32_t len, component_id components[len]);
entity_id ecs_create_entity(archtype_id archetype);
void *ecs_get_entity_component(entity_id e_id, component_id c_id);
void ecs_destroy_entity(entity_id id);
query_id ecs_register_query(ecs_query query);

void ecs_register_system(query_id query, ecs_system ecs_system);
uint32_t ecs_query_it_count(query_it *it);
void *ecs_query_it_get_components(query_it *it, component_id id);
void ecs_update();