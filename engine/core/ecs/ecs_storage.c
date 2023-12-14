#include "ecs_storage.h"
#include "../caffeine_memory.h"

#include "ecs_storage_type.h"

static int _storage_get_component_index(const ecs_storage *const storage, component_id id);
static void _storage_resize(ecs_storage *const storage, uint32_t capacity);

ecs_storage ecs_storage_new(const component_id *const components_owning, const size_t *const component_sizes_owning, uint32_t components_count)
{

    ecs_storage storage = (ecs_storage){
        .component_sizes = (const size_t *)component_sizes_owning,
        .components = (const component_id *)components_owning,
    };

    storage.component_count = components_count;

    storage.entity_capacity = 4;
    storage.entities = (entity_id *)CFF_ALLOC(sizeof(entity_id) * storage.entity_capacity, "STORAGE");
    storage.entity_data = (void **)CFF_ALLOC(sizeof(void *) * components_count, "STORAGE COMPONENTS");

    for (size_t i = 0; i < components_count; i++)
    {
        size_t component_size = component_sizes_owning[i];
        void *buffer = CFF_ALLOC((uint64_t)(component_size * storage.entity_capacity), "STORAGE COMPONENTS ARRAY");
        storage.entity_data[i] = buffer;
    }

    storage.entity_count = 0;
    return storage;
}

void ecs_storage_release(const ecs_storage *const storage_owning)
{
    if (storage_owning == NULL)
        return;

    for (size_t i = 0; i < storage_owning->component_count; i++)
    {
        void *buffer = storage_owning->entity_data[i];
        CFF_RELEASE(buffer);
    }

    CFF_RELEASE(storage_owning->entity_data);
    CFF_RELEASE(storage_owning->entities);
    CFF_RELEASE(storage_owning->component_sizes);
    CFF_RELEASE(storage_owning->components);
}

int ecs_storage_add_entity(ecs_storage *const storage_mut_ref, entity_id entity)
{
    if (storage_mut_ref->entity_count == storage_mut_ref->entity_capacity)
        _storage_resize(storage_mut_ref, storage_mut_ref->entity_capacity * 2);

    uint32_t row = storage_mut_ref->entity_count;

    storage_mut_ref->entities[row] = entity;

    storage_mut_ref->entity_count++;

    return row;
}

entity_id ecs_storage_remove_entity(ecs_storage *const storage_mut_ref, int row)
{
    if (storage_mut_ref->entity_count == 0)
        return INVALID_ID;

    int last_entity = storage_mut_ref->entity_count - 1;

    if (last_entity == row)
    {
        storage_mut_ref->entity_count--;
        return INVALID_ID;
    }

    storage_mut_ref->entities[row] = storage_mut_ref->entities[last_entity];
    storage_mut_ref->entities[last_entity] = INVALID_ID;

    for (size_t i = 0; i < storage_mut_ref->component_count; i++)
    {
        size_t component_size = storage_mut_ref->component_sizes[i];
        void *from = (void *)((uintptr_t)storage_mut_ref->entity_data[i] + (uintptr_t)(component_size * last_entity));
        void *to = (void *)((uintptr_t)storage_mut_ref->entity_data[i] + (uintptr_t)(component_size * row));
        CFF_COPY(from, to, component_size);
    }

    storage_mut_ref->entity_count--;
    return storage_mut_ref->entities[row];
}

void ecs_storage_set_component(ecs_storage *const storage_mut_ref, int row, component_id component, const void *const data)
{
    int component_index = _storage_get_component_index(storage_mut_ref, component);
    if (component_index != -1)
    {
        size_t component_size = storage_mut_ref->component_sizes[component_index];
        void *to = (void *)((uintptr_t)storage_mut_ref->entity_data[component_index] + (uintptr_t)(component_size * row));
        CFF_COPY(data, to, component_size);
    }
}

void *ecs_storage_get_component(const ecs_storage *const storage_ref, int row, component_id component)
{
    int component_index = _storage_get_component_index(storage_ref, component);
    if (component_index != -1)
    {
        size_t component_size = storage_ref->component_sizes[component_index];
        void *data = (void *)((uintptr_t)storage_ref->entity_data[component_index] + (uintptr_t)(component_size * row));
        return data;
    }
    return NULL;
}

void *ecs_storage_get_component_list(const ecs_storage *const storage_ref, component_id component)
{
    int component_index = _storage_get_component_index(storage_ref, component);
    if (component_index != -1)
    {
        return storage_ref->entity_data[component_index];
    }
    return NULL;
}

entity_id *ecs_storage_get_enetities_ids(const ecs_storage *const storage_ref)
{
    if (storage_ref != NULL)
    {
        return storage_ref->entities;
    }
    return NULL;
}

uint32_t ecs_storage_count(const ecs_storage *const storage_ref)
{
    if (storage_ref != NULL)
        return storage_ref->entity_count;
    return 0;
}

static int _storage_get_component_index(const ecs_storage *const storage_ref, component_id id)
{
    for (size_t i = 0; i < storage_ref->component_count; i++)
    {
        if (storage_ref->components[i] == id)
            return i;
    }
    return -1;
}

static void _storage_resize(ecs_storage *const storage_mut_ref, uint32_t capacity)
{
    storage_mut_ref->entities = CFF_ARR_RESIZE(storage_mut_ref->entities, capacity);
    for (size_t i = 0; i < storage_mut_ref->component_count; i++)
    {
        size_t component_size = storage_mut_ref->component_sizes[i];
        void *ptr = storage_mut_ref->entity_data[i];

        storage_mut_ref->entity_data[i] = CFF_REALLOC(ptr, component_size * capacity);
    }

    storage_mut_ref->entity_capacity = capacity;
}