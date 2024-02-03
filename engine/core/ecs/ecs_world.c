#include <stdbool.h>
#include "ecs_world.h"
#include "ecs_component.h"
#include "ecs_archetype_index.h"
#include "ecs_storage.h"
#include "ecs_storage_index.h"
#include "ecs_entity_index.h"
#include "component_dependency.h"
#include "ecs_system_index.h"
#include "../caffeine_memory.h"

struct ecs_world
{
    component_index *components_owning;
    archetype_index *archetypes_owning;
    storage_index *storages_owning;
    component_dependency *dependencies_owning;
    entity_index *entities_owning;
    system_index *systems_owning;
};

static bool ecs_world_is_archetype_valid(const ecs_world *const world, archetype_id id, ecs_query *query);
static void ecs_world_setup_archetype(const ecs_world *const world_ref, archetype_id archetype_id);

ecs_world *ecs_world_new()
{

    component_index *components_owning = ecs_new_component_index(64);

    if (components_owning == NULL)
    {
        return NULL;
    }

    archetype_index *archetypes_owning = ecs_new_archetype_index(64);

    if (archetypes_owning == NULL)
    {
        ecs_release_component_index(components_owning);
        return NULL;
    }

    component_dependency *dependencies_owning = ecs_component_dependency_init(64);

    if (dependencies_owning == NULL)
    {
        ecs_release_archetype_index(archetypes_owning);
        ecs_release_component_index(components_owning);
        return NULL;
    }

    storage_index *storages_owning = ecs_storage_index_new(64);

    if (storages_owning == NULL)
    {
        ecs_component_dependency_release(dependencies_owning);
        ecs_release_archetype_index(archetypes_owning);
        ecs_release_component_index(components_owning);
        return NULL;
    }

    entity_index *entities_owning = ecs_entity_index_new(64);
    if (entities_owning == NULL)
    {
        ecs_storage_index_release(storages_owning);
        ecs_component_dependency_release(dependencies_owning);
        ecs_release_archetype_index(archetypes_owning);
        ecs_release_component_index(components_owning);
        return NULL;
    }

    system_index *systems_owning = ecs_system_index_new(storages_owning, 64);
    if (systems_owning == NULL)
    {
        ecs_entity_index_release(entities_owning);
        ecs_storage_index_release(storages_owning);
        ecs_component_dependency_release(dependencies_owning);
        ecs_release_archetype_index(archetypes_owning);
        ecs_release_component_index(components_owning);
        return NULL;
    }

    ecs_world *world_owning = (ecs_world *)CFF_ALLOC(sizeof(ecs_world), "WORLD");

    if (world_owning == NULL)
    {
        ecs_entity_index_release(entities_owning);
        ecs_storage_index_release(storages_owning);
        ecs_component_dependency_release(dependencies_owning);
        ecs_release_archetype_index(archetypes_owning);
        ecs_release_component_index(components_owning);
        return NULL;
    }

    *world_owning = (ecs_world){
        .components_owning = components_owning,
        .archetypes_owning = archetypes_owning,
        .storages_owning = storages_owning,
        .dependencies_owning = dependencies_owning,
        .entities_owning = entities_owning,
        .systems_owning = systems_owning,
    };

    return world_owning;
}

void ecs_world_release(const ecs_world *const world_owning)
{
    ecs_system_index_release(world_owning->systems_owning);
    ecs_entity_index_release(world_owning->entities_owning);
    ecs_storage_index_release(world_owning->storages_owning);
    ecs_component_dependency_release(world_owning->dependencies_owning);
    ecs_release_archetype_index(world_owning->archetypes_owning);
    ecs_release_component_index(world_owning->components_owning);
    CFF_RELEASE(world_owning);
}

void ecs_world_step(const ecs_world *const world_ref, double delta_time)
{
    ecs_system_step(world_ref->systems_owning, delta_time);
}

#pragma region COMPONENT

component_id ecs_world_get_component(const ecs_world *const world_ref, const char *name)
{
    return ecs_get_component_id(world_ref->components_owning, name);
}

component_id ecs_world_add_component(const ecs_world *const world_ref, const char *name, size_t size, size_t align)
{
    return ecs_register_component(world_ref->components_owning, name, size, align);
}

void ecs_world_remove_component(const ecs_world *const world_ref, component_id id)
{
    ecs_remove_component(world_ref->components_owning, id);
    ecs_component_dependency_remove_component(world_ref->dependencies_owning, id);
}

#pragma endregion

#pragma region ARCHETYPE

archetype_id ecs_world_add_archetype(const ecs_world *const world_ref, ecs_archetype archetype)
{
    archetype_id archetype_id = ecs_register_archetype(world_ref->archetypes_owning, archetype);

    ecs_world_setup_archetype(world_ref, archetype_id);
    return archetype_id;
}

void ecs_world_remove_archetype(const ecs_world *const world_ref, archetype_id id)
{
    ecs_remove_archetype(world_ref->archetypes_owning, id);
    const component_id *components = NULL;
    const component_id **components_ref = &components;
    uint32_t len = ecs_archetype_get_components(world_ref->archetypes_owning, id, components_ref);

    if (components != NULL)
    {
        for (size_t i = 0; i < len; i++)
        {
            component_id c = components[i];
            ecs_component_dependency_remove_dependency(world_ref->dependencies_owning, c, id);
        }
    }

    ecs_storage_index_remove(world_ref->storages_owning, id);
}

static bool ecs_world_is_archetype_valid(const ecs_world *const world_ref, archetype_id id, ecs_query *query_ref)
{
    const component_id *comps = ecs_query_get_components(query_ref);
    uint32_t comp_count = ecs_query_get_count(query_ref);
    for (size_t j = 0; j < comp_count; j++)
    {
        component_id comp_id = comps[j];
        if (!ecs_archetype_has_component(world_ref->archetypes_owning, id, comp_id))
        {
            return false;
        }
    }
    return true;
}

static void ecs_world_setup_archetype(const ecs_world *const world_ref, archetype_id archetype_id)
{
    const archetype_index *const archetype_index = world_ref->archetypes_owning;

    const component_id *components = NULL;
    uint32_t compoennts_len = ecs_archetype_get_components(archetype_index, archetype_id, &components);

    ecs_system_index_add_archetype(world_ref->systems_owning, archetype_id, components, compoennts_len);

    size_t *component_sizes = (size_t *)CFF_ALLOC(compoennts_len * sizeof(size_t), "STORAGE COMPONENTS SIZES");
    component_id *components_copy = (component_id *)CFF_ALLOC(compoennts_len * sizeof(component_id), "STORAGE COMPONENTS");
    const char **component_names = (const char **)CFF_ALLOC(compoennts_len * sizeof(const char *), "STORAGE COMPONENTS NAMES");

    for (size_t i = 0; i < compoennts_len; i++)
    {
        component_id component = components[i];
        ecs_component_dependency_add_dependency(world_ref->dependencies_owning, component, archetype_id);
        components_copy[i] = component;
        component_sizes[i] = ecs_get_component_size(world_ref->components_owning, component);
        component_names[i] = ecs_get_component_name(world_ref->components_owning, component);
    }

    ecs_storage_index_new_storage(world_ref->storages_owning, archetype_id, components_copy, component_sizes, component_names, compoennts_len);
}
#pragma endregion

#pragma region ENTITY

entity_id ecs_world_create_entity(const ecs_world *const world_ref, archetype_id arhcetype_id)
{
    entity_id entity_id = ecs_entity_index_new_entity(world_ref->entities_owning);
    ecs_storage *storage = ecs_storage_index_get(world_ref->storages_owning, arhcetype_id);
    int row = ecs_storage_add_entity(storage, entity_id);
    ecs_entity_index_set_entity(world_ref->entities_owning, entity_id, arhcetype_id, row, storage);
    return entity_id;
}

void ecs_world_destroy_entity(const ecs_world *const world_ref, entity_id id)
{
    entity_record record = ecs_entity_index_get_entity(world_ref->entities_owning, id);
    entity_id moved_entity = ecs_storage_remove_entity(record.storage, record.row);
    ecs_entity_index_remove_entity(world_ref->entities_owning, id);
    ecs_entity_index_set_entity(world_ref->entities_owning, moved_entity, record.row, record.archetype, record.storage);
}

void *ecs_world_get_entity_component(const ecs_world *const world_ref, entity_id entity, component_id component)
{
    entity_record record = ecs_entity_index_get_entity(world_ref->entities_owning, entity);
    return ecs_storage_get_component(record.storage, record.row, component);
}

void ecs_world_set_entity_component(const ecs_world *const world_ref, entity_id entity, component_id component, void *data)
{
    entity_record record = ecs_entity_index_get_entity(world_ref->entities_owning, entity);
    ecs_storage_set_component(record.storage, record.row, component, data);
}

void ecs_world_add_entity_component(const ecs_world *const world_ref, entity_id entity, component_id component)
{
    // get wich archetype the entity is
    const entity_index *const entity_index_ref = world_ref->entities_owning;
    entity_record record = ecs_entity_index_get_entity(entity_index_ref, entity);
    archetype_id current_archetype = record.archetype;

    // get what archetype result on add component to previus archetype
    archetype_index *const archetype_index_mut_ref = world_ref->archetypes_owning;
    archetype_id next_archetype = ecs_archetype_add_component(archetype_index_mut_ref, current_archetype, component);
    ecs_world_setup_archetype(world_ref, next_archetype);

    // get storages from both archetypes
    const storage_index *const storage_index_ref = world_ref->storages_owning;
    ecs_storage *current_storage = record.storage;
    ecs_storage *next_storage = ecs_storage_index_get(storage_index_ref, next_archetype);

    // move entity from one storage to other
    int new_row = ecs_storage_move_entity(current_storage, next_storage, entity, record.row);

    // update entity on index
    ecs_entity_index_set_entity(world_ref->entities_owning, entity, new_row, next_archetype, next_storage);
}

void ecs_world_remove_entity_component(const ecs_world *const world_ref, entity_id entity, component_id component)
{
    // get wich archetype the entity is
    const entity_index *const entity_index_ref = world_ref->entities_owning;
    entity_record record = ecs_entity_index_get_entity(entity_index_ref, entity);
    archetype_id current_archetype = record.archetype;

    // get what archetype result on add component to previus archetype
    archetype_index *const archetype_index_mut_ref = world_ref->archetypes_owning;
    archetype_id next_archetype = ecs_archetype_remove_component(archetype_index_mut_ref, current_archetype, component);

    // get storages from both archetypes
    const storage_index *const storage_index_ref = world_ref->storages_owning;
    ecs_storage *current_storage = record.storage;
    ecs_storage *next_storage = ecs_storage_index_get(storage_index_ref, next_archetype);

    // move entity from one storage to other
    int new_row = ecs_storage_move_entity(current_storage, next_storage, entity, record.row);

    // update entity on index
    ecs_entity_index_set_entity(world_ref->entities_owning, entity, new_row, next_archetype, next_storage);
}

#pragma endregion

#pragma region SYSTEM

void ecs_worl_register_system(const ecs_world *const world_ref, ecs_query *query_owning, ecs_system system)
{
    const component_id *comps = ecs_query_get_components(query_owning);
    uint32_t comp_count = ecs_query_get_count(query_owning);

    component_id min_deps = ecs_component_dependency_get_less_dependencies(world_ref->dependencies_owning, comps, comp_count);

    const archetype_id *dependencies = NULL;
    uint32_t dep_count = ecs_component_dependency_get_dependencies(world_ref->dependencies_owning, min_deps, &dependencies);

    archetype_id eleged[dep_count];
    uint32_t eleged_count = 0;

    for (size_t i = 0; i < dep_count; i++)
    {
        archetype_id arch_id = dependencies[i];
        if (ecs_world_is_archetype_valid(world_ref, arch_id, query_owning))
        {
            eleged[eleged_count] = arch_id;
            eleged_count++;
        }
    }

    ecs_system_index_add(world_ref->systems_owning, query_owning, eleged, eleged_count, system);
}

#pragma endregion
