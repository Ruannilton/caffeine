#pragma once

#include "../ds/caffeine_ds.h"
#include <stdint.h>

#define INVALID_ID 0

typedef uint32_t component_id;
typedef uint32_t archtype_id;
typedef uint32_t entity_id;

cff_arr_dcltype(archetype, component_id);

bool ecs_init();
bool ecs_end();

component_id ecs_register_component(const char *name, uint32_t size);
archtype_id ecs_register_archetype(archetype arch);
archtype_id ecs_archetype_get(uint32_t len, component_id components[len]);
entity_id ecs_create_entity(archtype_id archetype);
void *ecs_get_entity_component(entity_id e_id, component_id c_id);
void ecs_destroy_entity(entity_id id);