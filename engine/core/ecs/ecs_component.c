#include "ecs_component.h"
#include <stdint.h>

#include "../caffeine_memory.h"

typedef struct
{
    component_id id;
    const char *name;
    size_t size;
    size_t align;
} component_info;

struct component_index
{
    component_info *data;
    uint32_t count;
    uint32_t capacity;
};

component_index *ecs_new_component_index(uint32_t capacity)
{
    component_index *instance = (component_index *)cff_mem_alloc(sizeof(component_index));

    if (instance == NULL)
        return instance;

    instance->capacity = capacity;
    instance->count = 0;
    instance->data = cff_mem_alloc(capacity * sizeof(component_info));

    if (instance->data == NULL)
    {
        cff_mem_release(instance);
        return NULL;
    }

    cff_mem_zero(instance->data, sizeof(component_info), capacity * sizeof(component_info));

    return instance;
}

component_id ecs_register_component(component_index *index, const char *name, size_t size, size_t align)
{
    component_id id = index->count;

    if (index->count == index->capacity)
    {
        index->data = cff_resize_arr(index->data, index->capacity * 2);
        index->capacity *= 2;
    }

    component_info info = {
        .align = align,
        .id = id,
        .name = name,
        .size = size,
    };

    index->data[id] = info;
    index->count++;
    return id;
}

size_t ecs_get_component_size(component_index *index, component_id id)
{
    if (id >= index->count)
        return 0;
    return index->data[id].size;
}

size_t ecs_get_component_align(component_index *index, component_id id)
{
    if (id >= index->count)
        return 0;
    return index->data[id].align;
}

void ecs_remove_component(component_index *index, component_id id)
{
    index->data[id] = (component_info){0};
}
// fix mem leak
void ecs_release_component_index(component_index *index)
{
    // cff_mem_zero(index->data, sizeof(component_info), index->capacity * sizeof(component_info));
    cff_mem_release(index->data);
    *index = (component_index){0};
    cff_mem_release(index);
}