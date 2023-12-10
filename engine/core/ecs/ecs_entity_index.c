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

    entity_index *index = (entity_index *)CFF_ALLOC(sizeof(entity_index), "ENTITY INDEX");

    if (index == NULL)
        return NULL;

    index->data = (entity_record *)CFF_ALLOC(sizeof(entity_record) * capacity, "ENTITY INDEX DATA");
    index->capacity = capacity;
    index->count = 0;

    index->trash = (entity_id *)CFF_ALLOC(sizeof(entity_id) * (capacity / 2), "ENTITY INDEX TRASH");
    index->trash_count = 0;
    index->trash_capacity = capacity / 2;

    return index;
}

void ecs_entity_index_release(entity_index *index)
{
    if (index == NULL)
        return;

    CFF_RELEASE(index->trash);
    CFF_RELEASE(index->data);
    CFF_RELEASE(index);
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
        index->data = CFF_ARR_RESIZE(index->data, index->capacity * 2);
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
        index->data[id] = (entity_record){.row = 0, .storage = 0};
    }

    if (index->trash_count == index->trash_capacity)
    {
        index->trash = CFF_ARR_RESIZE(index->trash, index->trash_capacity * 2);
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
    return (entity_record){.row = 0, .storage = 0};
}