#include <stdbool.h>
#include "ecs_world.h"
#include "ecs_component.h"
#include "ecs_archetype.h"
#include "ecs_storage.h"
#include "ecs_storage_index.h"
#include "ecs_entity_index.h"
#include "component_dependency.h"
#include "../caffeine_memory.h"

struct ecs_world
{
    component_index *components;
    archetype_index *archetypes;
    storage_index *storages;
    component_dependency *dependencies;
    entity_index *entities;
};

static bool is_archetype_valid(ecs_world *world, archetype_id id, ecs_query *query);

ecs_world *ecs_world_new()
{
    ecs_world *world = cff_mem_alloc(sizeof(ecs_world));
    if (world == NULL)
        return NULL;

    world->components = ecs_new_component_index(64);

    if (world->components == NULL)
    {
        cff_mem_release(world);
        return NULL;
    }

    world->archetypes = ecs_new_archetype_index(64);

    if (world->archetypes == NULL)
    {
        ecs_release_component_index(world->components);
        cff_mem_release(world);
        return NULL;
    }

    world->dependencies = ecs_component_dependency_init(64);
    if (world->dependencies == NULL)
    {
        ecs_release_archetype_index(world->archetypes);
        ecs_release_component_index(world->components);
        cff_mem_release(world);
        return NULL;
    }

    world->storages = ecs_storage_index_new(64);
    if (world->dependencies == NULL)
    {
        ecs_component_dependency_release(world->dependencies);
        ecs_release_archetype_index(world->archetypes);
        ecs_release_component_index(world->components);
        cff_mem_release(world);
        return NULL;
    }

    world->entities = ecs_entity_index_new(64);
    if (world->entities == NULL)
    {
        ecs_storage_index_release(world->storages);
        ecs_component_dependency_release(world->dependencies);
        ecs_release_archetype_index(world->archetypes);
        ecs_release_component_index(world->components);
        cff_mem_release(world);
        return NULL;
    }
    return world;
}

void ecs_world_release(ecs_world *world)
{
    ecs_entity_index_release(world->entities);
    ecs_storage_index_release(world->storages);
    ecs_component_dependency_release(world->dependencies);
    ecs_release_archetype_index(world->archetypes);
    ecs_release_component_index(world->components);
    cff_mem_release(world);
    *world = (ecs_world){0};
}

component_id ecs_world_add_component(ecs_world *world, const char *name, size_t size, size_t align)
{
    return ecs_register_component(world->components, name, size, align);
}

void ecs_world_remove_component(ecs_world *world, component_id id)
{
    ecs_remove_component(world->components, id);
    ecs_component_dependency_remove_component(world->dependencies, id);
}

archetype_id ecs_world_add_archetype(ecs_world *world, ecs_archetype archetype)
{
    archetype_id id = ecs_register_archetype(world->archetypes, archetype.count, archetype.components);

    size_t *component_sizes = (size_t *)cff_mem_alloc(archetype.count * sizeof(size_t));
    component_id *components = (component_id *)cff_mem_alloc(archetype.count * sizeof(component_id));

    for (size_t i = 0; i < archetype.count; i++)
    {
        component_id c = archetype.components[i];
        ecs_component_dependency_add_dependency(world->dependencies, c, id);
        components[i] = c;
        component_sizes[i] = ecs_get_component_size(world->components, c);
    }

    ecs_storage *storage = ecs_storage_new(components, component_sizes, archetype.count);
    ecs_storage_index_set(world->storages, id, storage);

    return id;
}

void ecs_world_remove_archetype(ecs_world *world, archetype_id id)
{
    ecs_remove_archetype(world->archetypes, id);
    component_id *components = NULL;
    uint32_t len = ecs_archetype_get_components(world->archetypes, id, components);

    for (size_t i = 0; i < len; i++)
    {
        component_id c = components[i];
        ecs_component_dependency_remove_dependency(world->dependencies, c, id);
    }

    ecs_storage *storage = ecs_storage_index_get(world->storages, id);
    ecs_storage_release(storage);
    ecs_storage_index_remove(world->storages, id);
}

entity_id ecs_world_create_entity(ecs_world *world, archetype_id id)
{
    entity_id e_id = ecs_entity_index_new_entity(world->entities);
    ecs_storage *storage = ecs_storage_index_get(world->storages, id);
    int row = ecs_storage_add_entity(storage, e_id);
    ecs_entity_index_set_entity(world->entities, e_id, row, storage);
    return e_id;
}

void ecs_world_destroy_entity(ecs_world *world, entity_id id)
{
    entity_record record = ecs_entity_index_get_entity(world->entities, id);
    entity_id moved_entity = ecs_storage_remove_entity(record.storage, record.row);
    ecs_entity_index_remove_entity(world->entities, id);
    ecs_entity_index_set_entity(world->entities, moved_entity, record.row, record.storage);
}

void *ecs_world_get_entity_component(ecs_world *world, entity_id entity, component_id component)
{
    entity_record record = ecs_entity_index_get_entity(world->entities, entity);
    return ecs_storage_get_component(record.storage, record.row, component);
}

void ecs_world_set_entity_component(ecs_world *world, entity_id entity, component_id component, void *data)
{
    entity_record record = ecs_entity_index_get_entity(world->entities, entity);
    ecs_storage_set_component(record.storage, record.row, component, data);
}

void ecs_worl_register_system(ecs_world *world, ecs_query *query, ecs_system system)
{
    component_id *comps = ecs_query_get_components(query);
    uint32_t comp_count = ecs_query_get_count(query);

    component_id min_deps = ecs_component_dependency_get_less_dependencies(world->dependencies, comps, comp_count);

    archetype_id *dependencies = NULL;
    uint32_t dep_count = ecs_component_dependency_get_dependencies(world->dependencies, min_deps, dependencies);

    archetype_id eleged[dep_count];
    uint32_t eleged_count = 0;

    for (size_t i = 0; i < dep_count; i++)
    {
        archetype_id arch_id = dependencies[i];
        if (is_archetype_valid(world, arch_id, query))
        {
            eleged[eleged_count] = arch_id;
            eleged_count++;
        }
    }
}

static bool is_archetype_valid(ecs_world *world, archetype_id id, ecs_query *query)
{
    component_id *comps = ecs_query_get_components(query);
    uint32_t comp_count = ecs_query_get_count(query);
    for (size_t j = 0; j < comp_count; j++)
    {
        component_id comp_id = comps[j];
        if (!ecs_archetype_has_component(world->archetypes, id, comp_id))
        {
            return false;
        }
    }
    return true;
}