#include "ecs_entity_index.h"

#include "../caffeine_memory.h"

struct entity_index
{
    uint32_t capacity;
    uint32_t count;
    entity_record *data;

    entity_id *trash;
    uint32_t trash_count;
    uint32_t trash_capacity;
};

entity_index *ecs_entity_index_new(uint32_t capacity)
{
    if (capacity < 4)
        capacity = 4;

    entity_index *index = (entity_index *)cff_mem_alloc(sizeof(entity_index));

    if (index == NULL)
        return NULL;

    index->data = (entity_record *)cff_mem_alloc(sizeof(entity_record) * capacity);
    index->capacity = capacity;
    index->count = 0;

    index->trash = (entity_id *)cff_mem_alloc(sizeof(entity_id) * (capacity / 2));
    index->trash_count = 0;
    index->trash_capacity = capacity / 2;

    return index;
}

void ecs_entity_index_release(entity_index *index)
{
    if (index == NULL)
        return;

    cff_mem_release(index->trash);
    cff_mem_release(index->data);
    cff_mem_release(index);
}

entity_id ecs_entity_index_new_entity(entity_index *index)
{
    if (index->trash_count > 0)
    {
        entity_id old_id = index->trash[index->trash_count - 1];
        index->trash_count--;
        return old_id;
    }

    if (index->count == index->capacity)
    {
        cff_resize_arr(index->data, index->capacity * 2);
        index->capacity *= 2;
    }
    entity_id id = (entity_id)index->count;
    index->count++;
    return id;
}

void ecs_entity_index_set_entity(entity_index *index, entity_id id, int row, ecs_storage *storage)
{
    if (id < index->capacity)
    {
        index->data[id] = (entity_record){.row = row, .storage = storage};
    }
}

void ecs_entity_index_remove_entity(entity_index *index, entity_id id)
{
    if (id < index->capacity)
    {
        index->data[id] = (entity_record){0};
    }

    if (index->trash_count == index->trash_capacity)
    {
        cff_resize_arr(index->trash, index->trash_capacity * 2);
        index->trash_capacity *= 2;
    }

    index->trash[index->trash_count] = id;
    index->trash_count++;
}

entity_record ecs_entity_index_get_entity(entity_index *index, entity_id id)
{
    if (id < index->capacity)
    {
        return index->data[id];
    }
    return (entity_record){0};
}