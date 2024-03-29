#include "ecs_component.h"
#include <stdint.h>

#include "../caffeine_memory.h"
#include "ecs_name_index.h"

typedef struct
{
    component_id id;
    const char *name;
    size_t size;
    size_t align;
} component_info;

struct component_index
{
    component_info *data_owning;
    uint32_t count;
    uint32_t capacity;
    name_index name_table;
};

component_index *ecs_new_component_index(uint32_t capacity)
{
    component_index *instance_owning = (component_index *)CFF_ALLOC(sizeof(component_index), "COMPONENT INDEX");

    if (instance_owning == NULL)
        return instance_owning;

    component_info *buffer_owning = (component_info *)CFF_ALLOC(capacity * sizeof(component_info), "COMPONENT INDEX BUFFER");

    if (buffer_owning == NULL)
    {
        CFF_RELEASE(instance_owning);
        return NULL;
    }

    CFF_ZERO(buffer_owning, capacity * sizeof(component_info));

    instance_owning->capacity = capacity;
    instance_owning->count = 0;
    instance_owning->data_owning = buffer_owning;

    ecs_name_index_init(&(instance_owning->name_table));

    return instance_owning;
}

component_id ecs_register_component(component_index *const index_mut_ref, const char *const name_ref, size_t size, size_t align)
{
    name_index *name_table = &(index_mut_ref->name_table);
    if (name_table == NULL)
    {
        return INVALID_ID;
    }

    component_id existent_id = INVALID_ID;
    ecs_name_index_get(name_table, (caff_string)name_ref, &existent_id);
    if (existent_id != INVALID_ID)
        return existent_id;

    component_id id = index_mut_ref->count;

    if (index_mut_ref->count == index_mut_ref->capacity)
    {
        index_mut_ref->data_owning = CFF_ARR_RESIZE(index_mut_ref->data_owning, index_mut_ref->capacity * 2);
        index_mut_ref->capacity *= 2;
    }

    component_info info = {
        .id = id,
        .name = name_ref,
        .size = size,
        .align = align,
    };

    index_mut_ref->data_owning[id] = info;
    index_mut_ref->count++;

    ecs_name_index_add(name_table, (caff_string)name_ref, id);

    return id;
}

component_id ecs_get_component_id(const component_index *const index_ref, const char *const name)
{
    caff_string name_str = (caff_string)name;
    const name_index *ni = &(index_ref->name_table);
    component_id existent_id = INVALID_ID;
    ecs_name_index_get(ni, name_str, &existent_id);
    return existent_id;
}

size_t ecs_get_component_size(const component_index *const index_ref, component_id id)
{
    if (id >= index_ref->count)
        return 0;
    return index_ref->data_owning[id].size;
}

size_t ecs_get_component_align(const component_index *const index_ref, component_id id)
{
    if (id >= index_ref->count)
        return 0;
    return index_ref->data_owning[id].align;
}

const char *const ecs_get_component_name(const component_index *const index_ref, component_id id)
{
    if (id >= index_ref->count)
        return NULL;
    return (const char *const)index_ref->data_owning[id].name;
}

void ecs_remove_component(component_index *const index_mut_ref, component_id id)
{
    caff_string name = (caff_string)index_mut_ref->data_owning[id].name;
    ecs_name_index_remove(&(index_mut_ref->name_table), name);

    index_mut_ref->data_owning[id] = (component_info){.id = 0, .name = 0, .size = 0, .align = 0};
}

void ecs_release_component_index(const component_index *const index_owning)
{
    const name_index *ni = &(index_owning->name_table);
    ecs_name_index_release(ni);
    CFF_RELEASE(index_owning->data_owning);
    CFF_RELEASE(index_owning);
}