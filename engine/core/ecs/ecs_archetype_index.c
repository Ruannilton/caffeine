#include "ecs_archetype_index.h"
#include <stdbool.h>
#include "../caffeine_memory.h"
#include "../caffeine_logging.h"
#include "../ds/caffeine_vector.h"
#include "../ds/caffeine_hashmap.h"

#define INVALID_INDEX (uint32_t)(0xffffffff)

cff_hash_dcltype(archetype_navigation, component_id, archetype_id);
cff_hash_impl(archetype_navigation, component_id, archetype_id);

struct archetype_info
{
    ecs_archetype archetype;
    archetype_navigation on_add;
    archetype_navigation on_remove;
};
typedef struct archetype_info archetype_info;
cff_hash_dcltype(archetype_reversed_map, ecs_archetype, archetype_id);
cff_hash_impl(archetype_reversed_map, ecs_archetype, archetype_id);

cff_hash_dcltype(archetype_map, archetype_id, archetype_info);
cff_hash_impl(archetype_map, archetype_id, archetype_info);

struct archetype_index
{
    archetype_map map_components_to_id;
    archetype_reversed_map map_id_to_components;
    uint32_t archetypes_generated;
};

static archetype_info archetype_info_create(ecs_archetype from);

static uint32_t archetype_map_hash_key_fn(archetype_id *key, uint32_t seed);
static bool archetype_map_cmp_key_fn(archetype_id *key_a, archetype_id *key_b);
static bool archetype_map_cmp_data_fn(archetype_info *data_a, archetype_info *data_b);

static uint32_t archetype_reversed_map_hash_key_fn(ecs_archetype *key, uint32_t seed);
static bool archetype_reversed_map_cmp_key_fn(ecs_archetype *key_a, ecs_archetype *key_b);
static bool archetype_reversed_map_cmp_data_fn(archetype_id *data_a, archetype_id *data_b);

static uint32_t archetype_navigation_hash_key_fn(component_id *key, uint32_t seed);
static bool archetype_navigation_cmp_key_fn(component_id *key_a, component_id *key_b);
static bool archetype_navigation_cmp_data_fn(archetype_id *data_a, archetype_id *data_b);

archetype_index *ecs_new_archetype_index(uint32_t capacity)
{
    archetype_index *instance = (archetype_index *)CFF_ALLOC(sizeof(archetype_index), "ARCHETYPE INDEX");

    if (instance == NULL)
    {
        caff_log_error("[ARCHETYPE INDEX] Creation error: fail to allocate archetype index memory\n");
        return instance;
    }

    archetype_map_init(
        &(instance->map_components_to_id),
        64,
        archetype_map_hash_key_fn,
        archetype_map_cmp_key_fn,
        archetype_map_cmp_data_fn);

    archetype_reversed_map_init(
        &(instance->map_id_to_components),
        64,
        archetype_reversed_map_hash_key_fn,
        archetype_reversed_map_cmp_key_fn,
        archetype_reversed_map_cmp_data_fn);

    instance->archetypes_generated = 0;

    return instance;
}

archetype_id ecs_register_archetype(archetype_index *const index_mut_ref, ecs_archetype archetype_owning)
{
    archetype_map *map_id_to_archetype = &index_mut_ref->map_components_to_id;
    archetype_reversed_map *map_archetype_to_id = &index_mut_ref->map_id_to_components;

    {
        archetype_id existent = INVALID_ID;
        if (archetype_reversed_map_get(map_archetype_to_id, archetype_owning, &existent))
        {
            return existent;
        }
    }

    archetype_id id = index_mut_ref->archetypes_generated;
    archetype_info info = archetype_info_create(archetype_owning);

    archetype_map_add(map_id_to_archetype, id, info);

    ecs_archetype reversed_key = ecs_archetype_copy(&archetype_owning);
    archetype_reversed_map_add(map_archetype_to_id, reversed_key, id);

    index_mut_ref->archetypes_generated++;
    caff_log_trace("[ARCHETYPE INDEX] Archetype registered: %" PRIu64 "\n", id);
    return id;
}

archetype_id ecs_get_archetype_id(const archetype_index *const index_ref, uint32_t count, const component_id *const components_ref)
{
    const archetype_reversed_map *const map_archetype_to_id = &index_ref->map_id_to_components;

    ecs_archetype arch = {
        .components = (component_id *)components_ref,
        .count = count,
        .capacity = count,
    };

    archetype_id existent = INVALID_ID;
    if (archetype_reversed_map_get(map_archetype_to_id, arch, &existent))
    {
        return existent;
    }

    caff_log_warn("[ARCHETYPE INDEX] Archetype not found\n");
    return INVALID_ID;
}

bool ecs_archetype_has_component(const archetype_index *const index_ref, archetype_id archetype, component_id component)
{
    const archetype_map *const map_id_to_archetype = &index_ref->map_components_to_id;
    archetype_info *info = NULL;

    bool exists = (bool)archetype_map_get_ref((archetype_map *)map_id_to_archetype, archetype, &info);

    if (!exists)
    {
        caff_log_error("[ARCHETYPE INDEX] Failed to search component: archetype id is invalid: %" PRIu64 "\n", archetype);
        return false;
    }

    return ecs_archetype_contains(&info->archetype, component);
}

void ecs_remove_archetype(archetype_index *const index_mut_ref, archetype_id id)
{
    archetype_map *map_id_to_archetype = &index_mut_ref->map_components_to_id;
    archetype_reversed_map *map_archetype_to_id = &index_mut_ref->map_id_to_components;
    archetype_info *info = NULL;

    bool exists = (bool)archetype_map_get_ref(map_id_to_archetype, id, &info);
    if (!exists || info == NULL)
    {
        return;
    }

    archetype_reversed_map_remove(map_archetype_to_id, info->archetype);
    archetype_map_remove(map_id_to_archetype, id);
}

void ecs_release_archetype_index(const archetype_index *const index_owning)
{
    if (index_owning == NULL)
    {
        caff_log_error("[ARCHETYPE INDEX] Failed to release archetype index: already null\n");
        return;
    }
    const archetype_map *const map_id_to_archetype = &index_owning->map_components_to_id;
    const archetype_reversed_map *const map_archetype_to_id = &index_owning->map_id_to_components;

    // fix mem leak, key and value should be properly released
    archetype_map_release(map_id_to_archetype);
    archetype_reversed_map_release(map_archetype_to_id);

    CFF_RELEASE(index_owning);
}

uint32_t ecs_archetype_get_components(const archetype_index *const index_ref, archetype_id id, const component_id **out_mut_ref)
{
    const archetype_map *const map_id_to_archetype = &index_ref->map_components_to_id;
    if (out_mut_ref == NULL)
    {
        caff_log_error("[ARCHETYPE INDEX] Failed to get components: out buffer is null\n");
        return 0;
    }
    archetype_info *info = NULL;
    if (archetype_map_get_ref((archetype_map *)map_id_to_archetype, id, &info))
    {
        *out_mut_ref = info->archetype.components;
        return info->archetype.count;
    }

    *out_mut_ref = NULL;
    return 0;
}

archetype_id ecs_archetype_add_component(archetype_index *const index_mut_ref, archetype_id origin_arch_id, component_id component)
{
    if (origin_arch_id == INVALID_ID)
    {
        caff_log_error("[ARCHETYPE INDEX] Failed to add component to archetype %" PRIu64 ": archetype id is invalid\n", origin_arch_id);
        return INVALID_ID;
    }

    if (component == INVALID_ID)
    {
        caff_log_error("[ARCHETYPE INDEX] Failed to add component to archetype %" PRIu64 ": archetype id is invalid\n", origin_arch_id);
        return INVALID_ID;
    }

    const archetype_map *const map_id_to_archetype = &index_mut_ref->map_components_to_id;
    archetype_info *from_arch_info = NULL;
    bool found = archetype_map_get_ref((archetype_map *)map_id_to_archetype, origin_arch_id, &from_arch_info);

    if (!found || from_arch_info == NULL)
        return INVALID_ID;

    ecs_archetype new_archetype = ecs_archetype_copy(&from_arch_info->archetype);
    ecs_archetype_add(&new_archetype, component);
    archetype_id new_archetype_id = ecs_register_archetype(index_mut_ref, new_archetype);

    archetype_info *new_arch_info = NULL;
    archetype_map_get_ref((archetype_map *)map_id_to_archetype, new_archetype_id, &new_arch_info);

    // make navigation
    archetype_navigation_add(&new_arch_info->on_remove, component, origin_arch_id);
    archetype_navigation_add(&from_arch_info->on_add, component, new_archetype_id);

    caff_log_trace("[ARCHETYPE INDEX] Component added to archetype %" PRIu64 ": result id is: %" PRIu64 "\n", origin_arch_id, new_archetype_id);
    return new_archetype_id;
}

archetype_id ecs_archetype_remove_component(archetype_index *const index_mut_ref, archetype_id origin_arch_id, component_id component)
{
    if (origin_arch_id == INVALID_ID)
    {
        caff_log_error("[ARCHETYPE INDEX] Failed to remove component %" PRIu64 ": archetype id %" PRIu64 " is invalid\n", component, origin_arch_id);
        return INVALID_ID;
    }
    if (component == INVALID_ID)
    {
        caff_log_error("[ARCHETYPE INDEX] Failed to remove component %" PRIu64 ": id is invalid\n", component);
        return INVALID_ID;
    }

    const archetype_map *const map_id_to_archetype = &index_mut_ref->map_components_to_id;
    archetype_info *from_arch_info = NULL;
    bool found = archetype_map_get_ref((archetype_map *)map_id_to_archetype, origin_arch_id, &from_arch_info);

    if (!found || from_arch_info == NULL)
        return INVALID_ID;

    ecs_archetype new_archetype = ecs_archetype_copy(&from_arch_info->archetype);
    ecs_archetype_remove(&new_archetype, component);
    archetype_id new_archetype_id = ecs_register_archetype(index_mut_ref, new_archetype);

    archetype_info *new_arch_info = NULL;
    archetype_map_get_ref((archetype_map *)map_id_to_archetype, new_archetype_id, &new_arch_info);

    // make navigation
    archetype_navigation_add(&new_arch_info->on_add, component, origin_arch_id);
    archetype_navigation_add(&from_arch_info->on_remove, component, new_archetype_id);

    caff_log_trace("[ARCHETYPE INDEX] Component added to archetype %" PRIu64 ": result id is: %" PRIu64 "\n", origin_arch_id, new_archetype_id);
    return new_archetype_id;
}

#pragma region UTILS

static uint32_t archetype_map_hash_key_fn(archetype_id *key, uint32_t seed)
{
    return (uint32_t)((seed * 31) + (uint64_t)*key);
}

static bool archetype_map_cmp_key_fn(archetype_id *key_a, archetype_id *key_b)
{
    return *key_a == *key_b;
}

static bool archetype_map_cmp_data_fn(archetype_info *data_a, archetype_info *data_b)
{
    return ecs_archetype_equals(&data_a->archetype, &data_b->archetype);
}

static uint32_t archetype_reversed_map_hash_key_fn(ecs_archetype *key, uint32_t seed)
{
    uint32_t hash = seed;
    for (size_t i = 0; i < key->count; i++)
    {
        hash = (hash * 7919) + key->components[i];
    }
    return hash;
}

static bool archetype_reversed_map_cmp_key_fn(ecs_archetype *key_a, ecs_archetype *key_b)
{
    return ecs_archetype_equals(key_a, key_b);
}

static bool archetype_reversed_map_cmp_data_fn(archetype_id *data_a, archetype_id *data_b)
{
    return *data_a == *data_b;
}

static uint32_t archetype_navigation_hash_key_fn(component_id *key, uint32_t seed)
{
    return (uint32_t)((seed * 31) + (uint64_t)*key);
}

static bool archetype_navigation_cmp_key_fn(component_id *key_a, component_id *key_b)
{
    return *key_a == *key_b;
}

static bool archetype_navigation_cmp_data_fn(archetype_id *data_a, archetype_id *data_b)
{
    return *data_a == *data_b;
}

static archetype_info archetype_info_create(ecs_archetype from)
{
    archetype_info info = {
        .archetype = from,
        .on_add = {0},
        .on_remove = {0},
    };

    archetype_navigation_init(&info.on_add, 4, archetype_navigation_hash_key_fn, archetype_navigation_cmp_key_fn, archetype_navigation_cmp_data_fn);
    archetype_navigation_init(&info.on_remove, from.count, archetype_navigation_hash_key_fn, archetype_navigation_cmp_key_fn, archetype_navigation_cmp_data_fn);

    return info;
}

#pragma endregion