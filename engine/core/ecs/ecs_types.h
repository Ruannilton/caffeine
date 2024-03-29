#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../../caffeine_types.h"

#define MAX_NAME_LENGHT 128

typedef uint64_t component_id;
typedef uint64_t archetype_id;
typedef uint64_t entity_id;
typedef const struct ecs_storage *const query_it;
typedef struct ecs_query ecs_query;
typedef struct
{
    component_id *components;
    uint32_t count;
    uint32_t capacity;
} ecs_archetype;

extern const uint64_t INVALID_ID;

typedef void (*ecs_system)(query_it iterator, uint32_t lenght, double delta_time);

CAFF_API ecs_archetype ecs_create_archetype(uint32_t len);

CAFF_API void ecs_archetype_add(ecs_archetype *const arch_mut_ref, component_id id);
CAFF_API void ecs_archetype_remove(ecs_archetype *const arch_mut_ref, component_id id);

ecs_archetype ecs_archetype_copy(const ecs_archetype *const arch_ref);
