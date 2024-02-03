#include "ecs_archetype_index.h"
#include <stdbool.h>
#include "../caffeine_memory.h"
#include "../ds/caffeine_vector.h"

#define INVALID_INDEX (uint32_t)(0xffffffff)

typedef struct archetype_info archetype_info;

cff_arr_dcltype(archetype_info_arr, archetype_id);
cff_arr_impl(archetype_info_arr, archetype_id);

struct archetype_info
{
    uint32_t lenght;
    uint32_t capacity;
    component_id *components;
    archetype_info_arr on_add;
    archetype_info_arr on_remove;
};

struct archetype_index
{
    uint32_t *keys;
    uint32_t *reverse_keys;
    archetype_info *values;
    uint32_t count;
    uint32_t capacity;
    uint32_t max_collision;
};

static bool compare_archetype(uint32_t count, const component_id a[], const component_id b[]);
static uint32_t hash_archetype(uint32_t count, const component_id components[], uint32_t seed);
static inline uint32_t min(uint32_t a, uint32_t b);
static archetype_info *get_arch_info(const archetype_index *const index_ref, archetype_id arch_id);
static void set_archetype_navigation(archetype_info_arr *info_arr, archetype_id next_archetype, component_id component);

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

    return instance;
}

archetype_id ecs_register_archetype(archetype_index *const index_mut_ref, ecs_archetype archetype_owning)
{
    archetype_id id = index_mut_ref->count + 1;

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

    archetype_info_arr_init(&(info.on_add), 2);
    archetype_info_arr_init(&(info.on_remove), info.lenght);
    archetype_info_arr_zero(&(info.on_remove));
    archetype_info_arr_zero(&(info.on_add));

    index_mut_ref->values[id] = info;
    index_mut_ref->keys[hash] = id;
    index_mut_ref->reverse_keys[id] = hash;
    index_mut_ref->count++;
    return id;
}

archetype_id ecs_get_archetype_id(const archetype_index *const index_ref, uint32_t count, const component_id *const components_ref)
{
    uint32_t seed = 71;
    uint32_t hash = hash_archetype(count, components_ref, seed) % index_ref->capacity;
    uint32_t tries = 1;

    while (index_ref->keys[hash] != INVALID_INDEX)
    {
        archetype_info *info = index_ref->values + index_ref->keys[hash];
        if (compare_archetype(min(info->lenght, count), info->components, components_ref))
        {
            return index_ref->keys[hash];
        }

        seed *= 2;
        hash = hash_archetype(count, components_ref, seed) % index_ref->capacity;
        tries++;

        if (tries == index_ref->max_collision)
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
    for (size_t i = 0; i < index_owning->count; i++)
    {
        if (index_owning->keys[index_owning->reverse_keys[i]] != INVALID_INDEX)
        {
            archetype_info *info = index_owning->values + i;
            CFF_RELEASE(info->components);
            archetype_info_arr_release(&(info->on_add));
            archetype_info_arr_release(&(info->on_remove));
        }
    }

    CFF_RELEASE(index_owning->values);
    CFF_RELEASE(index_owning->keys);
    CFF_RELEASE(index_owning->reverse_keys);

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

archetype_id ecs_archetype_add_component(archetype_index *const index_mut_ref, archetype_id origin_arch_id, component_id component)
{
    if (origin_arch_id == INVALID_ID || component == INVALID_ID)
        return INVALID_ID;

    const archetype_info *origin_arch_info = get_arch_info(index_mut_ref, origin_arch_id);

    if (origin_arch_info == NULL)
        return INVALID_ID;

    archetype_id next_arch_id = origin_arch_info->on_add.buffer[component];

    if (next_arch_id == INVALID_ID || next_arch_id == 0)
    {
        ecs_archetype current = {.capacity = origin_arch_info->capacity, .count = origin_arch_info->lenght, .components = origin_arch_info->components};
        ecs_archetype new_arch = ecs_archetype_copy(&current);
        ecs_archetype_add(&new_arch, component);
        next_arch_id = ecs_register_archetype(index_mut_ref, new_arch);

        if (next_arch_id == INVALID_ID)
            return INVALID_ID;

        const archetype_info *next_arch_info = get_arch_info(index_mut_ref, next_arch_id);

        archetype_info_arr *on_add_navigator = (archetype_info_arr *)&(origin_arch_info->on_add);
        archetype_info_arr *on_remove_navigator = (archetype_info_arr *)&(next_arch_info->on_remove);

        set_archetype_navigation(on_add_navigator, next_arch_id, component);
        set_archetype_navigation(on_remove_navigator, origin_arch_id, component);
    }

    return next_arch_id;
}

archetype_id ecs_archetype_remove_component(archetype_index *const index_mut_ref, archetype_id origin_arch_id, component_id component)
{
    if (origin_arch_id == INVALID_ID || component == INVALID_ID)
        return INVALID_ID;

    const archetype_info *origin_arch_info = get_arch_info(index_mut_ref, origin_arch_id);

    if (origin_arch_info == NULL)
        return INVALID_ID;

    archetype_id next_arch_id = origin_arch_info->on_remove.buffer[component];

    if (next_arch_id == INVALID_ID)
    {
        ecs_archetype current = {.capacity = origin_arch_info->capacity, .count = origin_arch_info->lenght, .components = origin_arch_info->components};
        ecs_archetype new_arch = ecs_archetype_copy(&current);
        ecs_archetype_remove(&new_arch, component);
        next_arch_id = ecs_register_archetype(index_mut_ref, new_arch);

        if (next_arch_id == INVALID_ID)
            return INVALID_ID;

        const archetype_info *next_arch_info = get_arch_info(index_mut_ref, next_arch_id);

        archetype_info_arr *on_remove_navigator = (archetype_info_arr *)&(origin_arch_info->on_remove);
        archetype_info_arr *on_add_navigator = (archetype_info_arr *)&(next_arch_info->on_add);

        set_archetype_navigation(on_add_navigator, next_arch_id, component);
        set_archetype_navigation(on_remove_navigator, origin_arch_id, component);
    }

    return next_arch_id;
}

#pragma region UTILS

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

static archetype_info *get_arch_info(const archetype_index *const index_ref, archetype_id arch_id)
{
    archetype_info *arch_info = &(index_ref->values[arch_id]);

    return arch_info;
}

static void set_archetype_navigation(archetype_info_arr *info_arr, archetype_id next_archetype, component_id component)
{
    if (info_arr->capacity < next_archetype)
    {
        uint32_t new_len = (uint32_t)(next_archetype + 1);
        archetype_info_arr_resize(info_arr, new_len);
    }
    for (size_t i = info_arr->count; i < info_arr->capacity; i++)
    {
        archetype_info_arr_set(info_arr, 0, i);
    }
    archetype_info_arr_set(info_arr, next_archetype, component);
}

#pragma endregion