#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef uint64_t component_id;
typedef uint64_t archetype_id;
typedef uint64_t entity_id;
typedef struct query_iterator query_it;

extern const uint64_t INVALID_ID;

typedef void (*ecs_system)(query_it iterator, uint32_t lenght);

typedef struct
{
    component_id *components;
    uint32_t count;
    uint32_t capacity;
} ecs_archetype;

ecs_archetype ecs_create_archetype(uint32_t len);

void ecs_archetype_add(ecs_archetype *const arch_mut_ref, component_id id);

typedef struct ecs_query ecs_query;

ecs_query *ecs_query_new(int count, const component_id *const components_ref);
const component_id *ecs_query_get_components(const ecs_query *const query_ref);
uint32_t ecs_query_get_count(const ecs_query *const query_ref);
void ecs_query_release(const ecs_query *const query_owning);