#include "component_dependency.h"

#include "../ds/caffeine_vector.h"

cff_arr_impl(dependency_list, archetype_id);
cff_hash_impl(component_dependency, component_id, dependency_list);

// cff_arr_impl(component_dependency, dependency_list);

static uint32_t hash_key_fn(component_id *id_ref, uint32_t seed)
{
    return (uint32_t)(*id_ref);
}

static bool cmp_key_fn(component_id *id_a_ref, component_id *id_b_ref)
{
    return *id_a_ref == *id_b_ref;
}

static bool cmp_data_fn(dependency_list *value_a_ref, dependency_list *value_b_ref)
{
    return value_a_ref->buffer == value_b_ref->buffer;
}

component_dependency *ecs_component_dependency_init(uint32_t capacity)
{
    component_dependency *cp_owning = cff_mem_alloc(sizeof(component_dependency));

    if (cp_owning == NULL)
        return NULL;

    component_dependency_init(cp_owning, capacity, hash_key_fn, cmp_key_fn, cmp_data_fn);

    return cp_owning;
}

void ecs_component_dependency_release(component_dependency *ptr)
{
    for (size_t i = 0; i < ptr->capacity; i++)
    {
        if (ptr->used_slot[i])
        {
            dependency_list *list = &(ptr->data_buffer[i]);
            dependency_list_release(list);
        }
    }

    component_dependency_release(ptr);
    cff_mem_release(ptr);
}

void ecs_component_dependency_add_component(component_dependency *ptr, component_id component)
{
    if (component_dependency_exist(ptr, component))
    {
        return;
    }

    dependency_list list = {0};
    dependency_list_init(&list, 4);

    component_dependency_add(ptr, component, list);
}

void ecs_component_dependency_remove_component(component_dependency *ptr, component_id component)
{
    dependency_list *list = NULL;
    if (component_dependency_get_ref(ptr, component, &list))
    {
        dependency_list_zero(list);
        dependency_list_release(list);
    }
    component_dependency_remove(ptr, component);
}

void ecs_component_dependency_add_dependency(component_dependency *ptr, component_id component, archetype_id archetype)
{

    dependency_list *list = NULL;

    if (component_dependency_get_ref(ptr, component, &list))
    {
        dependency_list_add(list, archetype);
        return;
    }

    ecs_component_dependency_add_component(ptr, component);

    if (component_dependency_get_ref(ptr, component, &list))
    {
        dependency_list_add(list, archetype);
    }
}

void ecs_component_dependency_remove_dependency(component_dependency *ptr, component_id component, archetype_id archetype)
{
    dependency_list *list = NULL;

    if (component_dependency_get_ref(ptr, component, &list))
    {
        dependency_list_remove(list, archetype);
    }
}

uint32_t ecs_component_dependency_get_dependencies(component_dependency *ptr, component_id component, const archetype_id **out)
{
    dependency_list *list = NULL;

    if (component_dependency_get_ref(ptr, component, &list))
    {
        *out = list->buffer;
        return list->count;
    }

    *out = NULL;
    return 0;
}

component_id ecs_component_dependency_get_less_dependencies(component_dependency *ptr, const component_id *const components, uint32_t len)
{
    uint32_t min_deps = 0xffffffff;
    component_id min_idx = 0xffffffff;

    for (uint32_t i = 0; i < len; i++)
    {
        component_id comp = components[i];
        dependency_list *list = NULL;

        if (component_dependency_get_ref(ptr, comp, &list))
        {
            if (list->capacity > 0)
            {
                if (list->count < min_deps)
                {
                    min_deps = list->count;
                    min_idx = comp;
                }
            }
        }
    }

    return min_idx;
}
component_id ecs_component_dependency_get_max_dependencies(component_dependency *ptr, const component_id *components, uint32_t len)
{
    uint32_t max_deps = 0;
    component_id max_idx = 0;

    for (uint32_t i = 0; i < len; i++)
    {
        component_id comp = components[i];
        dependency_list *list = NULL;

        if (component_dependency_get_ref(ptr, comp, &list))
        {
            if (list->capacity > 0)
            {
                if (list->count > max_deps)
                {
                    max_deps = list->count;
                    max_idx = comp;
                }
            }
        }
    }

    return max_idx;
}