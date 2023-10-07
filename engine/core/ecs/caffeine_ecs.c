#include "../caffeine_ecs.h"
#include "archetype.h"
#include "archetype_map.h"
#include "component_list.h"
#include "archetype_graph.h"
#include <stdarg.h>

#define LOG_SCOPE "ECS - "

bool caff_ecs_init()
{
    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Initializing ECS system\n");

    if (!component_list_init())
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to allocate component buffer\n");
        return false;
    }

    if (!map_init(8))
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to allocate archetype buffer\n");
        return false;
    }

    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Initialized\n");
    return true;
}

bool caff_ecs_end()
{

    component_list_end();
    map_release();

    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Shutdown\n");
    return true;
}

component_id caff_ecs_add_register_component(uint32_t component_size)
{
    component_id new_component = component_list_add(component_size);

    if (new_component == INVALID_COMPONENT_ID)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to register new component\n");
        caffeine_event_fire(EVENT_QUIT, (cff_event_data){0});
        return INVALID_COMPONENT_ID;
    }

    return new_component;
}

archetype_id caff_ecs_archetype_new(int components_count, ...)
{
    archetype arch;
    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "New archetype\n");
    int capacity = components_count;

    if (capacity < 4)
        capacity = 4;

    if (!archetype_init_sized(&arch, capacity))
        return INVALID_ARCHETYPE_ID;

    va_list args;

    va_start(args, components_count);

    for (int i = 0; i < components_count; i++)
    {
        component_id id = va_arg(args, component_id);
        if (!archetype_add(&arch, id))
        {
            archetype_release(&arch);
            va_end(args);
            caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Failed to create archetype\n");
            return INVALID_ARCHETYPE_ID;
        }
    }

    va_end(args);

    archetype_id id = INVALID_ARCHETYPE_ID;

    map_add(arch, &id);
    return id;
}

bool caff_ecs_archetype_add(archetype_id arch_id, component_id component)
{
    archetype *arch = map_get(arch_id);
    if (arch == NULL)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to find archetype with id: %u\n", arch_id);
        return false;
    }

    return archetype_add(arch, component);
}

bool caff_ecs_archetype_remove(archetype_id arch_id, component_id component)
{
    archetype *arch = map_get(arch_id);

    if (arch == NULL)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to find archetype with id: %u\n", arch_id);
        return false;
    }

    return archetype_remove(arch, component);
}

bool caff_ecs_build_world()
{
    return map_build();
}

entity_id caff_ecs_entity_new(archetype_id arch_id)
{
    return graph_alloc_entity(arch_id);
}

void *caff_ecs_entity_get_component(entity_id entity_id, component_id component_id)
{
    return graph_get_entity_component(entity_id, component_id);
}