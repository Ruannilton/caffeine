#include "component_list.h"

#define LOG_SCOPE "ECS::Component List - "

typedef struct
{
    uint32_t capacity, count;
    uint32_t *component_sizes;
} component_list;

static component_list list;

static bool _component_list_resize()
{
    uint32_t new_capacity = list.capacity * 2;
    list.component_sizes = cff_resize_arr(list.component_sizes, new_capacity);

    if (list.component_sizes == NULL)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to realloc size buffer\n");
        return false;
    }
    else
    {
        list.capacity = new_capacity;
    }
    return true;
}

bool component_list_init()
{
    list.capacity = 4;
    list.count = 0;
    list.component_sizes = cff_new_arr(uint32_t, list.capacity);

    if (list.component_sizes == NULL)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to alloc size buffer\n");
        return false;
    }

    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Initialized\n");
    return true;
}

component_id component_list_add(uint32_t size)
{
    component_id new_component = list.count;

    if (list.count == list.capacity)
        if (!_component_list_resize())
        {
            caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to add new component\n");
            return INVALID_COMPONENT_ID;
        }

    list.component_sizes[new_component] = size;
    list.count++;

    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Added component %u with size %u\n", new_component, size);
    return new_component;
}

uint32_t get_component_size(component_id id)
{
    if (list.count <= id)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Component id %u is invalid\n");
        return 0;
    }
    return list.component_sizes[id];
}

void component_list_end()
{

    if (list.component_sizes)
    {
        cff_mem_release((void *)list.component_sizes);
    }
    list = (component_list){0};

    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Released\n");
}
