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
    archetype_index *instance = (archetype_index *)cff_mem_alloc(sizeof(archetype_index));

    if (instance == NULL)
        return instance;

    instance->capacity = capacity;
    instance->count = 0;
    instance->max_collision = 0;
    instance->values = (archetype_info *)cff_mem_alloc(capacity * sizeof(archetype_info));

    if (instance->values == NULL)
    {
        cff_mem_release(instance);
        return NULL;
    }

    instance->keys = (uint32_t *)cff_mem_alloc(capacity * sizeof(uint32_t));
    if (instance->keys == NULL)
    {
        cff_mem_release(instance->values);
        cff_mem_release(instance);
        return NULL;
    }

    instance->reverse_keys = (uint32_t *)cff_mem_alloc(capacity * sizeof(uint32_t));
    if (instance->reverse_keys == NULL)
    {
        cff_mem_release(instance->keys);
        cff_mem_release(instance->values);
        cff_mem_release(instance);
        return NULL;
    }

    uint32_t init_val = INVALID_INDEX;
    cff_mem_zero(instance->values, capacity * sizeof(archetype_info));
    cff_mem_set(&init_val, instance->keys, sizeof(uint32_t), capacity * sizeof(uint32_t));
    cff_mem_set(&init_val, instance->reverse_keys, sizeof(uint32_t), capacity * sizeof(uint32_t));

    instance->graph = ecs_archetype_graph_new();

    return instance;
}

archetype_id ecs_register_archetype(archetype_index *const index, ecs_archetype archetype_owning)
{
    archetype_id id = index->count;

    uint32_t seed = 71;
    uint32_t hash = hash_archetype(archetype_owning.count, archetype_owning.components, seed) % index->capacity;
    uint32_t colision_count = 0;

    while (index->keys[hash] != INVALID_INDEX)
    {
        archetype_info *info = index->values + index->keys[hash];
        if (compare_archetype(min(info->lenght, archetype_owning.count), info->components, archetype_owning.components))
        {
            return index->keys[hash];
        }
        seed *= 2;
        hash = hash_archetype(archetype_owning.count, archetype_owning.components, seed) % index->capacity;
        colision_count++;
    }

    if (index->max_collision < colision_count)
        index->max_collision = colision_count;

    archetype_info info = {
        .lenght = archetype_owning.count,
        .capacity = archetype_owning.count,
        .components = archetype_owning.components,
    };

    index->values[id] = info;
    index->keys[hash] = id;
    index->reverse_keys[id] = hash;
    index->count++;

    ecs_archetype_graph_add(index->graph, id, archetype_owning.count, archetype_owning.components);

    return id;
}

archetype_id ecs_get_archetype_id(const archetype_index *const index, uint32_t count, const component_id *const components)
{
    uint32_t seed = 71;
    uint32_t hash = hash_archetype(count, components, seed) % index->capacity;
    uint32_t tries = 1;

    while (index->keys[hash] != INVALID_INDEX)
    {
        archetype_info *info = index->values + index->keys[hash];
        if (compare_archetype(min(info->lenght, count), info->components, components))
        {
            return index->keys[hash];
        }

        seed *= 2;
        hash = hash_archetype(count, components, seed) % index->capacity;
        tries++;

        if (tries == index->max_collision)
            break;
    }

    return INVALID_ID;
}

bool ecs_archetype_has_component(const archetype_index *const index, archetype_id archetype, component_id component)
{
    archetype_info *info = index->values + archetype;
    for (size_t i = 0; i < info->lenght; i++)
    {
        if (info->components[i] == component)
            return true;
    }

    return false;
}

void ecs_archetype_add_component(archetype_index *const index, archetype_id archetype, component_id component)
{
    if (ecs_archetype_has_component(index, archetype, component))
        return;

    archetype_info *info = index->values + archetype;

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

void ecs_archetype_remove_component(archetype_index *const index, archetype_id archetype, component_id component)
{
    if (!ecs_archetype_has_component(index, archetype, component))
        return;

    archetype_info *info = index->values + archetype;
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

void ecs_remove_archetype(archetype_index *const index, archetype_id id)
{
    if (index->count == 0)
        return;

    if (id >= index->count)
        return;

    uint32_t hash = index->reverse_keys[id];
    index->keys[hash] = INVALID_INDEX;
    index->reverse_keys[id] = INVALID_INDEX;
    index->values[id] = (archetype_info){
        .lenght = 0,
        .capacity = 0,
        .components = NULL,
    };
    index->count--;
}

// fix mem leak
void ecs_release_archetype_index(const archetype_index *const index)
{
    ecs_archetype_graph_release(index->graph);

    for (size_t i = 0; i < index->count; i++)
    {
        if (index->keys[index->reverse_keys[i]] != INVALID_INDEX)
        {
            archetype_info *info = index->values + i;
            cff_mem_release(info->components);
        }
    }

    cff_mem_release(index->values);
    cff_mem_release(index->keys);
    cff_mem_release(index->reverse_keys);

    // uint32_t init_val = (uint32_t)-1;
    // cff_mem_zero(index->values, sizeof(archetype_info), index->capacity * sizeof(archetype_info));
    // cff_mem_set(&init_val, index->keys, sizeof(uint32_t), index->capacity * sizeof(uint32_t));
    // cff_mem_set(&init_val, index->reverse_keys, sizeof(uint32_t), index->capacity * sizeof(uint32_t));

    cff_mem_release(index);
}

uint32_t ecs_archetype_get_components(archetype_index *index, archetype_id id, const component_id **out)
{
    if (index->reverse_keys[id] != INVALID_INDEX)
    {
        *out = index->values[id].components;
        return index->values[id].lenght;
    }

    *out = NULL;
    return 0;
}

static uint32_t hash_archetype(uint32_t count, const component_id components[], uint32_t seed)
{
    uint32_t hash_value = seed;

    for (size_t i = 0; i < count; i++)
    {
        // Mix the bits of the current integer into the hash value
        hash_value ^= components[i];
        hash_value *= 0x9e3779b1; // A prime number for mixing
    }

    return hash_value;
}

static bool compare_archetype(uint32_t count, const component_id a[], const component_id b[])
{
    for (size_t i = 0; i < count; i++)
    {
        if (a[i] != b[i])
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