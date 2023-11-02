#include "component_dependency.h"

#include "../ds/caffeine_vector.h"

cff_arr_impl(dependency_list, archetype_id);
cff_arr_impl(component_dependency, dependency_list);

component_dependency *ecs_component_dependency_init(uint32_t capacity)
{
    component_dependency *cp_dep = cff_mem_alloc(sizeof(component_dependency));

    if (cp_dep == NULL)
        return NULL;

    component_dependency_init(cp_dep, capacity);

    cff_mem_zero(cp_dep->buffer, sizeof(dependency_list), sizeof(dependency_list) * capacity);

    for (size_t i = 0; i < capacity; i++)
    {
        dependency_list *list = component_dependency_get_ref(cp_dep, i);
        list->capacity = 0;
        list->count = 0;
        list->buffer = 0;
    }

    return cp_dep;
}

void ecs_component_dependency_release(component_dependency *ptr)
{
    for (size_t i = 0; i < ptr->capacity; i++)
    {
        dependency_list *list = component_dependency_get_ref(ptr, i);
        dependency_list_release(list);
    }

    cff_mem_zero(ptr->buffer, sizeof(dependency_list), sizeof(dependency_list) * ptr->capacity);
    component_dependency_release(ptr);
    *ptr = (component_dependency){0};
    cff_mem_release(ptr);
}

void ecs_component_dependency_add_component(component_dependency *ptr, component_id component)
{
    if (component > ptr->capacity)
        component_dependency_resize(ptr, component + 1);

    if (component >= ptr->count)
        ptr->count = component + 1;

    dependency_list *list = component_dependency_get_ref(ptr, component);

    if (list->capacity == 0)
        dependency_list_init(list, 4);
}

void ecs_component_dependency_remove_component(component_dependency *ptr, component_id component)
{
    dependency_list *list = component_dependency_get_ref(ptr, component);
    dependency_list_zero(list);
    dependency_list_release(list);
    *list = (dependency_list){0};
}

void ecs_component_dependency_add_dependency(component_dependency *ptr, component_id component, archetype_id archetype)
{
    ecs_component_dependency_add_component(ptr, component);
    dependency_list *list = component_dependency_get_ref(ptr, component);
    dependency_list_add(list, archetype);
}

void ecs_component_dependency_remove_dependency(component_dependency *ptr, component_id component, archetype_id archetype)
{
    dependency_list *list = component_dependency_get_ref(ptr, component);
    if (list->count > 0)
        dependency_list_remove(list, archetype);
}

uint32_t ecs_component_dependency_get_dependencies(component_dependency *ptr, component_id component, const archetype_id *out)
{
    dependency_list *list = component_dependency_get_ref(ptr, component);
    if (list->count > 0)
    {
        out = list->buffer;
        return list->count;
    }
    out = NULL;
    return 0;
}

component_id ecs_component_dependency_get_less_dependencies(component_dependency *ptr, const component_id *components, uint32_t len)
{
    uint32_t min_deps = 0xffffffff;
    component_id min_idx = 0xffffffff;

    for (uint32_t i = 0; i < len; i++)
    {
        component_id comp = components[i];
        dependency_list *list = component_dependency_get_ref(ptr, comp);
        if (list->capacity > 0)
        {
            if (list->count < min_deps)
            {
                min_deps = list->count;
                min_idx = comp;
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
        dependency_list *list = component_dependency_get_ref(ptr, comp);
        if (list->capacity > 0)
        {
            if (list->count > max_deps)
            {
                max_deps = list->count;
                max_idx = comp;
            }
        }
    }

    return max_idx;
}