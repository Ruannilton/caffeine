#include "component_dependency.h"

#include "../ds/caffeine_vector.h"
#include "../caffeine_logging.h"

cff_arr_impl(dependency_list, archetype_id);
cff_hash_impl(component_dependency, component_id, dependency_list);

// cff_arr_impl(component_dependency, dependency_list);

static uint32_t hash_key_fn(component_id *id_ref, uint32_t seed)
{
    component_id id = *id_ref;
    uint32_t id_index = component_id_index(id);
    uint32_t hash = (uint32_t)(id_index) << seed;
    return hash;
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
    component_dependency *cp_owning = (component_dependency *)CFF_ALLOC(sizeof(component_dependency), "COMPONENT DEPENDENCY");

    if (cp_owning == NULL)
    {
        caff_log_error("[COMPONENT DEPENDENCY] Failed to init component dependency: memory not allocated\n");
        return NULL;
    }

    component_dependency_init(cp_owning, capacity, hash_key_fn, cmp_key_fn, cmp_data_fn);

    return cp_owning;
}

void ecs_component_dependency_release(const component_dependency *const ptr_owning)
{
    if (ptr_owning == NULL)
    {
        caff_log_error("[COMPONENT DEPENDENCY] Failed to release component dependency: pointer was null\n");
        return;
    }

    for (size_t i = 0; i < ptr_owning->capacity; i++)
    {
        if (ptr_owning->used_slot[i])
        {
            dependency_list *list = &(ptr_owning->data_buffer[i]);
            dependency_list_release(list);
        }
    }

    component_dependency_release((component_dependency *)ptr_owning);
    CFF_RELEASE(ptr_owning);
}

void ecs_component_dependency_add_component(component_dependency *const ptr_mut_ref, component_id component)
{
    if (component_dependency_exist(ptr_mut_ref, component))
    {
        caff_log_warn("[COMPONENT DEPENDENCY] Dependency list for component %" PRIu64 " not registered, already exists\n", component);
        return;
    }

    dependency_list list = {0};
    dependency_list_init(&list, 4);

    component_dependency_add(ptr_mut_ref, component, list);
    caff_log_trace("[COMPONENT DEPENDENCY] Dependency list for component %" PRIu64 " registered\n", component);
}

void ecs_component_dependency_remove_component(component_dependency *const ptr_mut_ref, component_id component)
{
    if (ptr_mut_ref == NULL)
    {
        caff_log_error("[COMPONENT DEPENDENCY] Failed to remove component %" PRIu64 ": pointer was null\n", component);
        return;
    }

    dependency_list *list = NULL;
    if (component_dependency_get_ref(ptr_mut_ref, component, &list))
    {
        dependency_list_zero(list);
        dependency_list_release(list);
    }
    component_dependency_remove(ptr_mut_ref, component);
}

void ecs_component_dependency_add_dependency_for_component(component_dependency *const ptr_mut_ref, component_id component, archetype_id archetype)
{
    if (ptr_mut_ref == NULL)
    {
        caff_log_error("[COMPONENT DEPENDENCY] Failed to add component %" PRIu64 ": pointer was null\n", component);
        return;
    }

    dependency_list *list = NULL;

    if (component_dependency_get_ref(ptr_mut_ref, component, &list))
    {
        dependency_list_add(list, archetype);
        caff_log_trace("[COMPONENT DEPENDENCY] Added archetype: %" PRIu64 " as dependent of component: %" PRIu64 "\n", archetype, component);
        return;
    }

    ecs_component_dependency_add_component(ptr_mut_ref, component);

    if (component_dependency_get_ref(ptr_mut_ref, component, &list))
    {
        dependency_list_add(list, archetype);
        caff_log_trace("[COMPONENT DEPENDENCY] Added archetype: %" PRIu64 " as dependent of component: %" PRIu64 "\n", archetype, component);
    }
}

void ecs_component_dependency_remove_dependency(component_dependency *const ptr_mut_ref, component_id component, archetype_id archetype)
{
    dependency_list *list = NULL;

    if (component_dependency_get_ref(ptr_mut_ref, component, &list))
    {
        dependency_list_remove(list, archetype);
        caff_log_trace("[COMPONENT DEPENDENCY] Removed archetype: %" PRIu64 " as dependent of component: %" PRIu64 "\n", archetype, component);
        return;
    }
    caff_log_warn("[COMPONENT DEPENDENCY] Archetype: %" PRIu64 " not removed from dependency list of component: %" PRIu64 "\n", archetype, component);
}

uint32_t ecs_component_dependency_get_dependencies(const component_dependency *const ptr_ref, component_id component, const archetype_id **out_mut_ref)
{
    dependency_list *list = NULL;

    if (component_dependency_get_ref((component_dependency *)ptr_ref, component, &list))
    {
        *out_mut_ref = list->buffer;
        return list->count;
    }

    caff_log_warn("[COMPONENT DEPENDENCY] Not found dependency list for component: %" PRIu64 "\n", component);

    *out_mut_ref = NULL;
    return 0;
}

component_id ecs_component_dependency_get_less_dependencies(const component_dependency *const ptr_ref, const component_id *const components, uint32_t len)
{
    uint32_t min_deps = 0xffffffff;
    component_id min_idx = 0xffffffff;

    for (uint32_t i = 0; i < len; i++)
    {
        component_id comp = components[i];
        dependency_list *list = NULL;

        if (component_dependency_get_ref((component_dependency *)ptr_ref, comp, &list))
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
component_id ecs_component_dependency_get_max_dependencies(const component_dependency *const ptr_ref, const component_id *const components_ref, uint32_t len)
{
    uint32_t max_deps = 0;
    component_id max_idx = 0;

    for (uint32_t i = 0; i < len; i++)
    {
        component_id comp = components_ref[i];
        dependency_list *list = NULL;

        if (component_dependency_get_ref((component_dependency *)ptr_ref, comp, &list))
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