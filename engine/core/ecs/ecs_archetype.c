#include "ecs_archetype.h"
#include <stdbool.h>
#include "../caffeine_memory.h"
#include "ecs_archetype_graph.h"

typedef struct
{
    uint32_t lenght;
    uint32_t capacity;
    component_id *components;
} archetype_info;

struct archetype_index
{
    uint32_t *map;
    uint32_t *reverse_map;
    archetype_info *data;
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
    instance->data = cff_mem_alloc(capacity * sizeof(archetype_info));

    if (instance->data == NULL)
    {
        cff_mem_release(instance);
        return NULL;
    }

    instance->map = cff_mem_alloc(capacity * sizeof(uint32_t));
    if (instance->map == NULL)
    {
        cff_mem_release(instance->data);
        cff_mem_release(instance);
        return NULL;
    }

    instance->reverse_map = cff_mem_alloc(capacity * sizeof(uint32_t));
    if (instance->reverse_map == NULL)
    {
        cff_mem_release(instance->map);
        cff_mem_release(instance->data);
        cff_mem_release(instance);
        return NULL;
    }

    uint32_t init_val = (uint32_t)-1;
    cff_mem_zero(instance->data, sizeof(archetype_info), capacity * sizeof(archetype_info));
    cff_mem_set(&init_val, instance->map, sizeof(uint32_t), capacity * sizeof(uint32_t));
    cff_mem_set(&init_val, instance->reverse_map, sizeof(uint32_t), capacity * sizeof(uint32_t));

    instance->graph = ecs_archetype_graph_new();

    return instance;
}

archetype_id ecs_register_archetype(archetype_index *index, uint32_t count, component_id components[])
{
    archetype_id id = index->count;

    uint32_t seed = 71;
    uint32_t hash = hash_archetype(count, components, seed) % index->capacity;
    uint32_t colision_count = 0;

    while (index->map[hash] != (uint32_t)-1)
    {
        archetype_info *info = index->data + index->map[hash];
        if (compare_archetype(min(info->lenght, count), info->components, components))
        {
            return index->map[hash];
        }
        seed *= 2;
        hash = hash_archetype(count, components, seed) % index->capacity;
        colision_count++;
    }

    if (index->max_collision < colision_count)
        index->max_collision = colision_count;

    archetype_info info = {
        .components = components,
        .lenght = count,
        .capacity = count,
    };

    index->data[id] = info;
    index->map[hash] = id;
    index->reverse_map[id] = hash;
    index->count++;

    ecs_archetype_graph_add(index->graph, id, count, components);

    return id;
}

archetype_id ecs_get_archetype_id(archetype_index *index, uint32_t count, component_id components[])
{
    uint32_t seed = 71;
    uint32_t hash = hash_archetype(count, components, seed) % index->capacity;
    uint32_t tries = 1;

    while (index->map[hash] != (uint32_t)-1)
    {
        archetype_info *info = index->data + index->map[hash];
        if (compare_archetype(min(info->lenght, count), info->components, components))
        {
            return index->map[hash];
        }

        seed *= 2;
        hash = hash_archetype(count, components, seed) % index->capacity;
        tries++;

        if (tries == index->max_collision)
            break;
    }

    return INVALID_ID;
}

bool ecs_archetype_has_component(archetype_index *index, archetype_id archetype, component_id component)
{
    archetype_info *info = index->data + archetype;
    for (size_t i = 0; i < info->lenght; i++)
    {
        if (info->components[i] == component)
            return true;
    }

    return false;
}

void ecs_archetype_add_component(archetype_index *index, archetype_id archetype, component_id component)
{
    if (ecs_archetype_has_component(index, archetype, component))
        return;

    archetype_info *info = index->data + archetype;
    if (info->lenght == info->capacity)
    {
        cff_resize_arr(info->components, info->capacity * 2);
        info->capacity *= 2;
    }

    uint32_t idx = -1;
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

void ecs_archetype_remove_component(archetype_index *index, archetype_id archetype, component_id component)
{
    if (!ecs_archetype_has_component(index, archetype, component))
        return;

    archetype_info *info = index->data + archetype;
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

void ecs_remove_archetype(archetype_index *index, archetype_id id)
{
    if (index->count == 0)
        return;

    uint32_t hash = index->reverse_map[id];
    index->map[hash] = -1;
    index->reverse_map[id] = -1;
    index->data[id] = (archetype_info){0};
    index->count--;
}
// fix mem leak
void ecs_release_archetype_index(archetype_index *index)
{
    ecs_archetype_graph_release(index->graph);

    for (size_t i = 0; i < index->count; i++)
    {
        if (index->map[index->reverse_map[i]] != -1)
        {
            archetype_info *info = index->data + i;
            cff_mem_release(info->components);
            cff_mem_zero(info->components, sizeof(component_id), info->capacity * sizeof(component_id));
        }
    }

    cff_mem_release(index->data);
    cff_mem_release(index->map);
    cff_mem_release(index->reverse_map);

    uint32_t init_val = (uint32_t)-1;
    cff_mem_zero(index->data, sizeof(archetype_info), index->capacity * sizeof(archetype_info));
    cff_mem_set(&init_val, index->map, sizeof(uint32_t), index->capacity * sizeof(uint32_t));
    cff_mem_set(&init_val, index->reverse_map, sizeof(uint32_t), index->capacity * sizeof(uint32_t));

    *index = (archetype_index){0};
    cff_mem_release(index);
}

uint32_t ecs_archetype_get_components(archetype_index *index, archetype_id id, const component_id *out)
{
    if (index->reverse_map[id] != -1)
    {
        out = index->data[id].components;
        return index->data[id].lenght;
    }

    out = NULL;
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