#pragma once

#include <stdint.h>
#include "caffeine_memory.h"
#include "caffeine_logging.h"
#include "caffeine_events.h"

typedef uint32_t component_id;
typedef uint32_t archetype_id;
typedef uint64_t entity_id;

#define INVALID_COMPONENT_ID ((component_id)-1)
#define INVALID_ARCHETYPE_ID ((archetype_id)-1)
#define INVALID_ENTITY_ID ((entity_id)-1)

typedef struct
{
    uint32_t capacity, count;
    component_id *buffer;
} archetype;

bool caff_ecs_init();
bool caff_ecs_end();

component_id caff_ecs_add_register_component(uint32_t component_size);

// INFO: ARCHETYPE
archetype_id caff_ecs_archetype_new(int components_count, ...);
bool caff_ecs_archetype_add(archetype_id arch_id, component_id component);
bool caff_ecs_archetype_remove(archetype_id arch_id, component_id component);

bool caff_ecs_build_world();

// INFO: ENTITY
entity_id caff_ecs_entity_new(archetype_id arch_id);
void *caff_ecs_entity_get_component(entity_id entity_id, component_id component_id);