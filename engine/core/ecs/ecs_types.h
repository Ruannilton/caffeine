#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "../../caffeine_types.h"

#define MAX_NAME_LENGHT 128

typedef union
{
    struct
    {
        uint32_t index;
        uint16_t scope;
        uint16_t flags;
    };
    uint64_t value;
} component_id_metadata;

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

typedef enum
{
    COMPONENT_REGULAR = 0,
    COMPONENT_TAG = ((uint16_t)1 << 15),
} component_type;

extern const uint64_t INVALID_ID;

typedef void (*ecs_system)(query_it iterator, uint32_t lenght, double delta_time);

CAFF_API ecs_archetype ecs_create_archetype(uint32_t len);

CAFF_API void ecs_archetype_add(ecs_archetype *const arch_mut_ref, component_id id);
CAFF_API void ecs_archetype_remove(ecs_archetype *const arch_mut_ref, component_id id);
CAFF_API bool ecs_archetype_contains(const ecs_archetype *const arch_ref, component_id id);
CAFF_API bool ecs_archetype_equals(const ecs_archetype *const arch_ref_a, const ecs_archetype *const arch_ref_b);

ecs_archetype ecs_archetype_copy(const ecs_archetype *const arch_ref);

inline component_id_metadata component_id_unpack(component_id id)
{
    return (*(component_id_metadata *)(&id));
}

inline component_id component_id_pack(component_id_metadata meta)
{
    return (*(component_id *)(&meta));
}

inline uint32_t component_id_index(component_id id)
{
    return (*(component_id_metadata *)(&id)).index;
}

inline uint32_t component_id_is_tag(component_id id)
{
    return ((*(component_id_metadata *)(&id)).flags & COMPONENT_TAG) != 0;
}