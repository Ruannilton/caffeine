#include "ecs_archetype.h"
#include <stdbool.h>
#include "../caffeine_memory.h"
#include "ecs_archetype_graph.h"

#define INVALID_INDEX (uint32_t)(0xffffffff)

typedef struct
{
    uint32_t lenght;
    uint32_t capacity;
    component_id *components;
} archetype_info;

struct archetype_index
{
    uint32_t *keys;
    uint32_t *reverse_keys;
    archetype_info *values;
    uint32_t count;
    uint32_t capacity;
    uint32_t max_collision;
    archetype_graph *graph;
};

static bool compare_archetype(uint32_t count, const component_id a[], const component_id b[]);

static uint32_t hash_archetype(uint32_t count, const component_id components[], uint32_t seed);

static inline uint32_t min(uint32_t a, uint32_t b);

archetype_index *ecs_new_archetype_index(uint32_t capacity)
{
    archetype_index *instance = (archetype_index *)CFF_ALLOC(sizeof(archetype_index), "ARCHETYPE INDEX");

    if (instance == NULL)
        return instance;

    instance->capacity = capacity;
    instance->count = 0;
    instance->max_collision = 0;
    instance->values = (archetype_info *)CFF_ALLOC(capacity * sizeof(archetype_info), "ARCHETYPE INDEX VALUES");

    if (instance->values == NULL)
    {
        CFF_RELEASE(instance);
        return NULL;
    }

    instance->keys = (uint32_t *)CFF_ALLOC(capacity * sizeof(uint32_t), "ARCHETYPE INDEX KEYS");
    if (instance->keys == NULL)
    {
        CFF_RELEASE(instance->values);
        CFF_RELEASE(instance);
        return NULL;
    }

    instance->reverse_keys = (uint32_t *)CFF_ALLOC(capacity * sizeof(uint32_t), "ARCHETYPE INDEX INVERSE KEYS");
    if (instance->reverse_keys == NULL)
    {
        CFF_RELEASE(instance->keys);
        CFF_RELEASE(instance->values);
        CFF_RELEASE(instance);
        return NULL;
    }

    uint32_t init_val = INVALID_INDEX;
    CFF_ZERO(instance->values, capacity * sizeof(archetype_info));
    CFF_SET(&init_val, instance->keys, sizeof(uint32_t), capacity * sizeof(uint32_t));
    CFF_SET(&init_val, instance->reverse_keys, sizeof(uint32_t), capacity * sizeof(uint32_t));

    instance->graph = ecs_archetype_graph_new();

    return instance;
}

archetype_id ecs_register_archetype(archetype_index *const index_mut_ref, ecs_archetype archetype_owning)
{
    archetype_id id = index_mut_ref->count;

    uint32_t seed = 71;
    uint32_t hash = hash_archetype(archetype_owning.count, archetype_owning.components, seed) % index_mut_ref->capacity;
    uint32_t colision_count = 0;

    while (index_mut_ref->keys[hash] != INVALID_INDEX)
    {
        archetype_info *info = index_mut_ref->values + index_mut_ref->keys[hash];
        if (compare_archetype(min(info->lenght, archetype_owning.count), info->components, archetype_owning.components))
        {
            return index_mut_ref->keys[hash];
        }
        seed *= 2;
        hash = hash_archetype(archetype_owning.count, archetype_owning.components, seed) % index_mut_ref->capacity;
        colision_count++;
    }

    if (index_mut_ref->max_collision < colision_count)
        index_mut_ref->max_collision = colision_count;

    archetype_info info = {
        .lenght = archetype_owning.count,
        .capacity = archetype_owning.count,
        .components = archetype_owning.components,
    };

    index_mut_ref->values[id] = info;
    index_mut_ref->keys[hash] = id;
    index_mut_ref->reverse_keys[id] = hash;
    index_mut_ref->count++;

    ecs_archetype_graph_add(index_mut_ref->graph, id, archetype_owning.count, archetype_owning.components);

    return id;
}

archetype_id ecs_get_archetype_id(archetype_index *const index_mut_ref, uint32_t count, const component_id *const components_ref)
{
    uint32_t seed = 71;
    uint32_t hash = hash_archetype(count, components_ref, seed) % index_mut_ref->capacity;
    uint32_t tries = 1;

    while (index_mut_ref->keys[hash] != INVALID_INDEX)
    {
        archetype_info *info = index_mut_ref->values + index_mut_ref->keys[hash];
        if (compare_archetype(min(info->lenght, count), info->components, components_ref))
        {
            return index_mut_ref->keys[hash];
        }

        seed *= 2;
        hash = hash_archetype(count, components_ref, seed) % index_mut_ref->capacity;
        tries++;

        if (tries == index_mut_ref->max_collision)
            break;
    }

    return INVALID_ID;
}

bool ecs_archetype_has_component(const archetype_index *const index_ref, archetype_id archetype, component_id component)
{
    archetype_info *info = index_ref->values + archetype;
    for (size_t i = 0; i < info->lenght; i++)
    {
        if (info->components[i] == component)
            return true;
    }

    return false;
}

void ecs_archetype_add_component(archetype_index *const index_mut_ref, archetype_id archetype, component_id component)
{
    if (ecs_archetype_has_component(index_mut_ref, archetype, component))
        return;

    archetype_info *info = index_mut_ref->values + archetype;

    if (info->lenght == info->capacity)
    {
        info->components = CFF_ARR_RESIZE(info->components, info->capacity * 2);
        info->capacity *= 2;
    }

    uint32_t idx = INVALID_INDEX;
    for (size_t i = 0; i < info->lenght; i++)
    {
        if (info->components[i] < component)
            continue;
        idx = 1;
        break;
    }

    for (size_t i = info->lenght - 1; i >= idx; i--)
    {
        info->components[i + 1] = info->components[i];
    }

    info->components[idx] = component;
    info->lenght++;
}

void ecs_archetype_remove_component(archetype_index *const index_mut_ref, archetype_id archetype, component_id component)
{
    if (!ecs_archetype_has_component(index_mut_ref, archetype, component))
        return;

    archetype_info *info = index_mut_ref->values + archetype;
    uint32_t idx = 0;

    for (size_t i = 0; i < info->lenght; i++)
    {
        if (info->components[i] == component)
        {
            idx = i;
            break;
        }
    }

    for (size_t i = idx; i < info->lenght; i++)
    {
        info->components[i] = info->components[i + 1];
    }

    info->lenght--;
}

void ecs_remove_archetype(archetype_index *const index_mut_ref, archetype_id id)
{
    if (index_mut_ref->count == 0)
        return;

    if (id >= index_mut_ref->count)
        return;

    uint32_t hash = index_mut_ref->reverse_keys[id];
    index_mut_ref->keys[hash] = INVALID_INDEX;
    index_mut_ref->reverse_keys[id] = INVALID_INDEX;
    index_mut_ref->values[id] = (archetype_info){
        .lenght = 0,
        .capacity = 0,
        .components = NULL,
    };
    index_mut_ref->count--;
}

// fix mem leak
void ecs_release_archetype_index(const archetype_index *const index_owning)
{
    ecs_archetype_graph_release(index_owning->graph);

    for (size_t i = 0; i < index_owning->count; i++)
    {
        if (index_owning->keys[index_owning->reverse_keys[i]] != INVALID_INDEX)
        {
            archetype_info *info = index_owning->values + i;
            CFF_RELEASE(info->components);
        }
    }

    CFF_RELEASE(index_owning->values);
    CFF_RELEASE(index_owning->keys);
    CFF_RELEASE(index_owning->reverse_keys);

    // uint32_t init_val = (uint32_t)-1;
    // CFF_ZERO(index->values, sizeof(archetype_info), index->capacity * sizeof(archetype_info));
    // CFF_SET(&init_val, index->keys, sizeof(uint32_t), index->capacity * sizeof(uint32_t));
    // CFF_SET(&init_val, index->reverse_keys, sizeof(uint32_t), index->capacity * sizeof(uint32_t));

    CFF_RELEASE(index_owning);
}

uint32_t ecs_archetype_get_components(const archetype_index *const index_ref, archetype_id id, const component_id **out_mut_ref)
{
    if (index_ref->reverse_keys[id] != INVALID_INDEX)
    {
        *out_mut_ref = index_ref->values[id].components;
        return index_ref->values[id].lenght;
    }

    *out_mut_ref = NULL;
    return 0;
}

static uint32_t hash_archetype(uint32_t count, const component_id *const components_ref, uint32_t seed)
{
    uint32_t hash_value = seed;

    for (size_t i = 0; i < count; i++)
    {
        // Mix the bits of the current integer into the hash value
        hash_value ^= components_ref[i];
        hash_value *= 0x9e3779b1; // A prime number for mixing
    }

    return hash_value;
}

static bool compare_archetype(uint32_t count, const component_id *const a_ref, const component_id *const b_ref)
{
    for (size_t i = 0; i < count; i++)
    {
        if (a_ref[i] != b_ref[i])
            return false;
    }
    return true;
}

static inline uint32_t min(uint32_t a, uint32_t b)
{
    if (a < b)
        return a;
    return b;
}