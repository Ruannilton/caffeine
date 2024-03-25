#include "ecs_archetype_index.h"
#include <stdbool.h>
#include "../caffeine_memory.h"
#include "../caffeine_logging.h"
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
static archetype_info *get_archetype_info(const archetype_index *const index_ref, archetype_id arch_id);
static void set_archetype_navigation(archetype_info_arr *info_arr, archetype_id next_archetype, component_id component);

archetype_index *ecs_new_archetype_index(uint32_t capacity)
{
    archetype_index *instance = (archetype_index *)CFF_ALLOC(sizeof(archetype_index), "ARCHETYPE INDEX");

    if (instance == NULL)
    {
        caff_log_error("[ARCHETYPE INDEX] Creation error: fail to allocate archetype index memory\n");
        return instance;
    }

    instance->capacity = capacity;
    instance->count = 0;
    instance->max_collision = 0;
    instance->values = (archetype_info *)CFF_ALLOC(capacity * sizeof(archetype_info), "ARCHETYPE INDEX VALUES");

    if (instance->values == NULL)
    {
        caff_log_error("[ARCHETYPE INDEX] Creation error: fail to allocate archetype index values memory\n");
        CFF_RELEASE(instance);
        return NULL;
    }

    instance->keys = (uint32_t *)CFF_ALLOC(capacity * sizeof(uint32_t), "ARCHETYPE INDEX KEYS");
    if (instance->keys == NULL)
    {
        caff_log_error("[ARCHETYPE INDEX] Creation error: fail to allocate archetype index keys memory\n");
        CFF_RELEASE(instance->values);
        CFF_RELEASE(instance);
        return NULL;
    }

    instance->reverse_keys = (uint32_t *)CFF_ALLOC(capacity * sizeof(uint32_t), "ARCHETYPE INDEX INVERSE KEYS");
    if (instance->reverse_keys == NULL)
    {
        caff_log_error("[ARCHETYPE INDEX] Creation error: fail to allocate archetype index reverse keys memory\n");
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
    archetype_id id = index_mut_ref->count;

    uint32_t seed = 71;
    uint32_t hash = hash_archetype(archetype_owning.count, archetype_owning.components, seed) % index_mut_ref->capacity;
    uint32_t colision_count = 0;

    while (index_mut_ref->keys[hash] != INVALID_INDEX)
    {
        archetype_info *info = index_mut_ref->values + index_mut_ref->keys[hash];
        if (compare_archetype(min(info->lenght, archetype_owning.count), info->components, archetype_owning.components))
        {
            archetype_id existent = index_mut_ref->keys[hash];
            caff_log_warn("[ARCHETYPE INDEX] Archetype already exists: %" PRIu64 "\n", existent);
            return existent;
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
    caff_log_trace("[ARCHETYPE INDEX] Archetype registered: %" PRIu64 "\n", id);
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
            archetype_id found = index_ref->keys[hash];
            caff_log_trace("[ARCHETYPE INDEX] Archetype found: %" PRIu64 "\n", found);
            return found;
        }

        seed *= 2;
        hash = hash_archetype(count, components_ref, seed) % index_ref->capacity;
        tries++;

        if (tries == index_ref->max_collision)
            break;
    }

    caff_log_warn("[ARCHETYPE INDEX] Archetype not found\n");
    return INVALID_ID;
}

bool ecs_archetype_has_component(const archetype_index *const index_ref, archetype_id archetype, component_id component)
{
    if (archetype >= index_ref->count || archetype == INVALID_ID)
    {
        caff_log_error("[ARCHETYPE INDEX] Failed to search component: archetype id is invalid: %" PRIu64 "\n", archetype);
        return false;
    }

    if (component == INVALID_ID)
    {
        caff_log_error("[ARCHETYPE INDEX] Failed to search component: component id is invalid: %" PRIu64 "\n", archetype);
        return false;
    }

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
    if (id >= index_mut_ref->count)
    {
        caff_log_error("[ARCHETYPE INDEX] Failed to remove archetype: id is invalid: %" PRIu64 "\n", id);
        return;
    }

    uint32_t hash = index_mut_ref->reverse_keys[id];
    index_mut_ref->keys[hash] = INVALID_INDEX;
    index_mut_ref->reverse_keys[id] = INVALID_INDEX;
    index_mut_ref->values[id] = (archetype_info){
        .lenght = 0,
        .capacity = 0,
        .components = NULL,
    };
    index_mut_ref->count--;

    caff_log_trace("[ARCHETYPE INDEX] Archetype removed, left: %" PRIu32 "\n", index_mut_ref->count);
}

// fix mem leak
void ecs_release_archetype_index(const archetype_index *const index_owning)
{
    if (index_owning == NULL)
    {
        caff_log_error("[ARCHETYPE INDEX] Failed to release archetype index: already null\n");
        return;
    }
    for (size_t i = 0; i < index_owning->count; i++)
    {
        uint32_t reverse_key = index_owning->reverse_keys[i];

        if (reverse_key >= index_owning->capacity)
            continue;

        if (index_owning->keys[reverse_key] != INVALID_INDEX)
        {
            archetype_info *info = index_owning->values + i;

            if (info->components != NULL && info->lenght > 0)
            {
                CFF_RELEASE(info->components);
            }

            archetype_info_arr_release(&(info->on_add));

            archetype_info_arr_release(&(info->on_remove));
        }
    }

    if (index_owning->values)
    {
        CFF_RELEASE(index_owning->values);
    }
    if (index_owning->keys)
    {
        CFF_RELEASE(index_owning->keys);
    }
    if (index_owning->reverse_keys)
    {
        CFF_RELEASE(index_owning->reverse_keys);
    }

    CFF_RELEASE(index_owning);
}

uint32_t ecs_archetype_get_components(const archetype_index *const index_ref, archetype_id id, const component_id **out_mut_ref)
{
    if (out_mut_ref == NULL)
    {
        caff_log_error("[ARCHETYPE INDEX] Failed to get components: out buffer is null\n");
        return 0;
    }

    if (id >= index_ref->count || index_ref->reverse_keys[id] > index_ref->capacity)
    {
        caff_log_error("[ARCHETYPE INDEX] Failed to get components: archetype id is invalid: %" PRIu64 "\n", id);
        *out_mut_ref = NULL;
        return 0;
    }

    *out_mut_ref = index_ref->values[id].components;
    return index_ref->values[id].lenght;
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

    const archetype_info *origin_arch_info = get_archetype_info(index_mut_ref, origin_arch_id);

    if (origin_arch_info == NULL)
    {
        caff_log_error("[ARCHETYPE INDEX] Failed to add component to archetype %" PRIu64 ": archetype not found\n", origin_arch_id);
        return INVALID_ID;
    }

    uint32_t component_index = component_id_index(component);
    archetype_id next_arch_id = INVALID_ID;

    if (component_index < origin_arch_info->on_add.capacity)
    {
        next_arch_id = origin_arch_info->on_add.buffer[component_index];
    }

    if (next_arch_id == INVALID_ID || next_arch_id == 0)
    {
        caff_log_trace("[ARCHETYPE INDEX] Target archetype does not exist creating new\n");

        ecs_archetype current = {
            .capacity = origin_arch_info->capacity,
            .count = origin_arch_info->lenght,
            .components = origin_arch_info->components,
        };

        ecs_archetype new_arch = ecs_archetype_copy(&current);

        ecs_archetype_add(&new_arch, component);

        next_arch_id = ecs_register_archetype(index_mut_ref, new_arch);

        if (next_arch_id == INVALID_ID)
        {
            caff_log_error("[ARCHETYPE INDEX] Failed to register new archetype while adding component %" PRIu64 " to archetype %" PRIu64 "\n", component, origin_arch_id);
            return INVALID_ID;
        }

        const archetype_info *next_arch_info = get_archetype_info(index_mut_ref, next_arch_id);
        if (next_arch_info == NULL)
        {
            caff_log_error("[ARCHETYPE INDEX] Failed to get new archetype info for id %" PRIu64 " while adding component %" PRIu64 " to archetype %" PRIu64 "\n", next_arch_id, component, origin_arch_id);
            return INVALID_ID;
        }

        archetype_info_arr *on_add_navigator = (archetype_info_arr *)&(origin_arch_info->on_add);

        archetype_info_arr *on_remove_navigator = (archetype_info_arr *)&(next_arch_info->on_remove);

        set_archetype_navigation(on_add_navigator, next_arch_id, component);
        set_archetype_navigation(on_remove_navigator, origin_arch_id, component);
    }

    caff_log_trace("[ARCHETYPE INDEX] Component added to archetype %" PRIu64 ": result id is: %" PRIu64 "\n", origin_arch_id, next_arch_id);
    return next_arch_id;
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

    const archetype_info *origin_arch_info = get_archetype_info(index_mut_ref, origin_arch_id);

    if (origin_arch_info == NULL)
    {
        caff_log_error("[ARCHETYPE INDEX] Failed to remove component %" PRIu64 ": archetype info not found\n", component);
        return INVALID_ID;
    }

    archetype_id next_arch_id = origin_arch_info->on_remove.buffer[component];

    if (next_arch_id == INVALID_ID)
    {
        caff_log_trace("[ARCHETYPE INDEX] Target archetype does not exist creating new\n");

        ecs_archetype current = {
            .capacity = origin_arch_info->capacity,
            .count = origin_arch_info->lenght,
            .components = origin_arch_info->components,
        };

        ecs_archetype new_arch = ecs_archetype_copy(&current);

        ecs_archetype_remove(&new_arch, component);

        next_arch_id = ecs_register_archetype(index_mut_ref, new_arch);

        if (next_arch_id == INVALID_ID)
        {
            caff_log_error("[ARCHETYPE INDEX] Failed to register new archetype while removing component %" PRIu64 " from archetype %" PRIu64 "\n", component, origin_arch_id);
            return INVALID_ID;
        }

        const archetype_info *next_arch_info = get_archetype_info(index_mut_ref, next_arch_id);
        if (next_arch_info == NULL)
        {
            caff_log_error("[ARCHETYPE INDEX] Failed to get new archetype info for id %" PRIu64 " while removing component %" PRIu64 " from archetype %" PRIu64 "\n", next_arch_id, component, origin_arch_id);
            return INVALID_ID;
        }

        archetype_info_arr *on_remove_navigator = (archetype_info_arr *)&(origin_arch_info->on_remove);
        archetype_info_arr *on_add_navigator = (archetype_info_arr *)&(next_arch_info->on_add);

        set_archetype_navigation(on_add_navigator, next_arch_id, component);
        set_archetype_navigation(on_remove_navigator, origin_arch_id, component);
    }

    caff_log_trace("[ARCHETYPE INDEX] Component removed from archetype %" PRIu64 ": result id is: %" PRIu64 "\n", origin_arch_id, next_arch_id);
    return next_arch_id;
}

#pragma region UTILS

static uint32_t hash_archetype(uint32_t count, const component_id *const components_ref, uint32_t seed)
{
    uint32_t hash_value = seed;

    for (size_t i = 0; i < count; i++)
    {
        component_id component = components_ref[i];
        uint32_t id_index = component_id_index(component);
        // Mix the bits of the current integer into the hash value
        hash_value ^= id_index;
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

static archetype_info *get_archetype_info(const archetype_index *const index_ref, archetype_id arch_id)
{
    if (arch_id >= index_ref->count)
    {
        caff_log_error("[ARCHETYPE INDEX] Failed to get archetype info with id: %" PRIu64 "\n");
        return NULL;
    }
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
    uint32_t component_index = component_id_index(component);
    archetype_info_arr_set(info_arr, next_archetype, component_index);
}

#pragma endregion