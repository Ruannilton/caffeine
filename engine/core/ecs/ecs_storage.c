#include "ecs_storage.h"
#include "../caffeine_memory.h"

#include "ecs_storage_type.h"

static int _storage_get_component_index(const ecs_storage *const storage, component_id id);
static void _storage_resize(ecs_storage *const storage, uint32_t capacity);

ecs_storage ecs_storage_new(component_id *components, size_t *component_sizes, uint32_t components_count)
{

    ecs_storage storage = (ecs_storage){0};

    storage.component_sizes = component_sizes;
    storage.components = components;
    storage.component_count = components_count;

    storage.entity_capacity = 4;
    storage.entities = (entity_id *)cff_mem_alloc(sizeof(entity_id) * storage.entity_capacity);
    storage.entity_data = (void **)cff_mem_alloc(sizeof(void *) * components_count);

    for (size_t i = 0; i < components_count; i++)
    {
        size_t component_size = component_sizes[i];
        void *buffer = cff_mem_alloc((uint64_t)(component_size * storage.entity_capacity));
        storage.entity_data[i] = buffer;
    }

    storage.entity_count = 0;
    return storage;
}

void ecs_storage_release(const ecs_storage *const storage)
{
    if (storage == NULL)
        return;

    for (size_t i = 0; i < storage->component_count; i++)
    {
        void *buffer = storage->entity_data[i];
        cff_mem_release(buffer);
    }

    cff_mem_release(storage->entity_data);
    cff_mem_release(storage->entities);
    cff_mem_release(storage->component_sizes);
    cff_mem_release(storage->components);
}

int ecs_storage_add_entity(ecs_storage *const storage, entity_id entity)
{
    if (storage->entity_count == storage->entity_capacity)
        _storage_resize(storage, storage->entity_capacity * 2);

    uint32_t row = storage->entity_count;

    storage->entities[row] = entity;

    storage->entity_count++;

    return row;
}

entity_id ecs_storage_remove_entity(ecs_storage *const storage, int row)
{
    if (storage->entity_count == 0)
        return INVALID_ID;

    int last_entity = storage->entity_count - 1;

    if (last_entity == row)
    {
        storage->entity_count--;
        return INVALID_ID;
    }

    storage->entities[row] = storage->entities[last_entity];
    storage->entities[last_entity] = INVALID_ID;

    for (size_t i = 0; i < storage->component_count; i++)
    {
        size_t component_size = storage->component_sizes[i];
        void *from = (void *)((uintptr_t)storage->entity_data[i] + (uintptr_t)(component_size * last_entity));
        void *to = (void *)((uintptr_t)storage->entity_data[i] + (uintptr_t)(component_size * row));
        cff_mem_copy(from, to, component_size);
    }

    storage->entity_count--;
    return storage->entities[row];
}

void ecs_storage_set_component(ecs_storage *const storage, int row, component_id component, void *data)
{
    int component_index = _storage_get_component_index(storage, component);
    if (component_index != -1)
    {
        size_t component_size = storage->component_sizes[component_index];
        void *to = (void *)((uintptr_t)storage->entity_data[component_index] + (uintptr_t)(component_size * row));
        cff_mem_copy(data, to, component_size);
    }
}

void *ecs_storage_get_component(ecs_storage *const storage, int row, component_id component)
{
    int component_index = _storage_get_component_index(storage, component);
    if (component_index != -1)
    {
        size_t component_size = storage->component_sizes[component_index];
        void *data = (void *)((uintptr_t)storage->entity_data[component_index] + (uintptr_t)(component_size * row));
        return data;
    }
    return NULL;
}

static int _storage_get_component_index(const ecs_storage *const storage, component_id id)
{
    for (size_t i = 0; i < storage->component_count; i++)
    {
        if (storage->components[i] == id)
            return i;
    }
    return -1;
}

static void _storage_resize(ecs_storage *const storage, uint32_t capacity)
{
    storage->entities = cff_resize_arr(storage->entities, capacity);
    for (size_t i = 0; i < storage->component_count; i++)
    {
        size_t component_size = storage->component_sizes[i];
        void *ptr = storage->entity_data[i];

        storage->entity_data[i] = cff_mem_realloc(ptr, component_size * capacity);
    }

    storage->entity_capacity = capacity;
}