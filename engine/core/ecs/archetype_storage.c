#include "archetype_storage.h"
#include "component_list.h"

#define LOG_SCOPE "ECS::Archetype Storage - "

static void _switch_entities(archetype_storage *storage, uint32_t index_a, uint32_t index_b)
{
    void *buffer = storage->move_buffer;

    // copy B data to buffer
    for (uint32_t i = 0; i < storage->storage_header.component_count; i++)
    {
        uint32_t component_size = storage->storage_header.component_sizes[i];

        void *component_data = (void *)(((uintptr_t)storage->component_datas[i]) + component_size * index_b);

        cff_mem_copy(component_data, buffer, (uint64_t)component_size);

        buffer = (void *)(((uintptr_t)buffer) + component_size);
    }

    // copy A data to B
    for (uint32_t i = 0; i < storage->storage_header.component_count; i++)
    {
        uint32_t component_size = storage->storage_header.component_sizes[i];

        void *component_data_a = (void *)(((uintptr_t)storage->component_datas[i]) + component_size * index_a);
        void *component_data_b = (void *)(((uintptr_t)storage->component_datas[i]) + component_size * index_b);

        cff_mem_copy(component_data_a, component_data_b, (uint64_t)component_size);
    }

    // copy buffer to A

    buffer = storage->move_buffer;
    for (uint32_t i = 0; i < storage->storage_header.component_count; i++)
    {
        uint32_t component_size = storage->storage_header.component_sizes[i];

        void *component_data_a = (void *)(((uintptr_t)storage->component_datas[i]) + component_size * index_a);
        cff_mem_copy(buffer, component_data_a, (uint64_t)component_size);

        buffer = (void *)((uintptr_t)buffer + component_size);
    }

    uint32_t entry_index_a = storage->entity_entry_reverse[index_a];
    uint32_t entry_index_b = storage->entity_entry_reverse[index_b];

    storage->entity_entry[entry_index_a] = index_b;
    storage->entity_entry[entry_index_b] = index_a;

    storage->entity_entry_reverse[index_b] = entry_index_a;
    storage->entity_entry_reverse[index_a] = entry_index_b;
}

bool ecs_storage_init(archetype_storage *storage, archetype_id id, archetype *archetype)
{
    uint32_t component_count = archetype->count;
    uint32_t capacity = 4;

    // HEADER
    storage->storage_header.component_count = component_count;

    storage->storage_header.component_sizes = cff_new_arr(uint32_t, component_count);
    storage->storage_header.arch_id = id;

    storage->storage_header.components_ids = cff_new_arr(component_id, component_count);

    storage->entity_capacity = capacity;

    // COMPONENT DATAS
    storage->component_datas = cff_new_arr(void *, component_count);
    if (storage->component_datas == NULL)
        return false;

    // ENTITY ENTRY
    storage->entity_entry = cff_new_arr(uint32_t, capacity);
    if (storage->entity_entry == NULL)
    {
        cff_mem_release(storage->component_datas);
        return false;
    }

    // ENTITY ENTRY REVERSE
    storage->entity_entry_reverse = cff_mem_alloc(sizeof(uint32_t) * capacity);
    if (storage->entity_entry_reverse == NULL)
    {
        cff_mem_release(storage->component_datas);
        cff_mem_release(storage->entity_entry);
        return false;
    }

#ifdef CFF_DEBUG
    cff_mem_zero(storage->component_datas, sizeof(void *), sizeof(void *) * component_count);
    cff_mem_zero(storage->entity_entry, sizeof(uint32_t), sizeof(uint32_t) * capacity);
    cff_mem_zero(storage->entity_entry_reverse, sizeof(uint32_t), sizeof(uint32_t) * capacity);
#endif

    uint32_t entity_size = 0;
    for (uint32_t i = 0; i < component_count; i++)
    {
        component_id comp_id = archetype->buffer[i];
        uint32_t comp_size = get_component_size(comp_id);
        entity_size += comp_size;
        storage->storage_header.component_sizes[i] = comp_size;
        storage->storage_header.components_ids[i] = comp_id;
        storage->component_datas[i] = cff_mem_alloc(comp_size * capacity);
        cff_mem_zero(storage->component_datas[i], comp_size, comp_size * capacity);
    }

    storage->move_buffer = cff_mem_alloc(entity_size);
    storage->entity_count = 0;
    storage->entity_entry_count = 0;

    return true;
}

void ecs_storage_release(archetype_storage *storage)
{
    uint32_t component_count = storage->storage_header.component_count;

    for (uint32_t i = 0; i < component_count; i++)
    {
        cff_mem_release(storage->component_datas[i]);
    }

    cff_mem_release(storage->storage_header.component_sizes);
    cff_mem_release(storage->storage_header.components_ids);
    cff_mem_release(storage->component_datas);
    cff_mem_release(storage->entity_entry);
    cff_mem_release(storage->entity_entry_reverse);
    cff_mem_release(storage->move_buffer);
}

// TODO: Handle resize
entity_id ecs_storage_create_entity(archetype_storage *storage)
{
    uint32_t index = storage->entity_count;
    uint32_t entry_index = storage->entity_entry_count;

    storage->entity_entry[entry_index] = index;
    storage->entity_entry_reverse[index] = entry_index;

    storage->entity_count++;
    storage->entity_entry_count++;

    return index;
}

void ecs_storage_destroy_entity(archetype_storage *storage, entity_id id)
{
    uint32_t entry_index = (uint32_t)(id & 0xFFFFFFFF);

    uint32_t index_to_remove = storage->entity_entry[entry_index];

    uint32_t index_to_swap = storage->entity_count - 1;

    _switch_entities(storage, index_to_remove, index_to_swap);
}

void *ecs_storage_get_entity_component(archetype_storage *storage, entity_id ent_id, component_id comp_id)
{
    for (uint32_t i = 0; i < storage->storage_header.component_count; i++)
    {
        component_id id = storage->storage_header.components_ids[i];
        if (id != comp_id)
            continue;

        uint32_t component_size = storage->storage_header.component_sizes[i];

        void *component_data = (void *)(((uintptr_t)storage->component_datas[i]) + component_size * ent_id);

        return component_data;
    }

    caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Node component %u found for entity: %u \n", comp_id, ent_id);
    return NULL;
}