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

void ecs_entity_index_release(const entity_index *const index_owning)
{
    if (index_owning == NULL)
        return;

    CFF_RELEASE(index_owning->trash);
    CFF_RELEASE(index_owning->data);
    CFF_RELEASE(index_owning);
}

entity_id ecs_entity_index_new_entity(entity_index *const index_mut_ref)
{
    if (index_mut_ref->trash_count > 0)
    {
        entity_id old_id = index_mut_ref->trash[index_mut_ref->trash_count - 1];
        index_mut_ref->trash_count--;
        return old_id;
    }

    if (index_mut_ref->count == index_mut_ref->capacity)
    {
        index_mut_ref->data = CFF_ARR_RESIZE(index_mut_ref->data, index_mut_ref->capacity * 2);
        index_mut_ref->capacity *= 2;
    }
    entity_id id = (entity_id)index_mut_ref->count;
    index_mut_ref->count++;
    return id;
}

void ecs_entity_index_set_entity(entity_index *const index_mut_ref, entity_id id, int row, ecs_storage *const storage_owning)
{
    if (id < index_mut_ref->capacity)
    {
        index_mut_ref->data[id] = (entity_record){.row = row, .storage = storage_owning};
    }
}

void ecs_entity_index_remove_entity(entity_index *const index_mut_ref, entity_id id)
{
    if (id < index_mut_ref->capacity)
    {
        index_mut_ref->data[id] = (entity_record){.row = 0, .storage = 0};
    }

    if (index_mut_ref->trash_count == index_mut_ref->trash_capacity)
    {
        index_mut_ref->trash = CFF_ARR_RESIZE(index_mut_ref->trash, index_mut_ref->trash_capacity * 2);
        index_mut_ref->trash_capacity *= 2;
    }

    index_mut_ref->trash[index_mut_ref->trash_count] = id;
    index_mut_ref->trash_count++;
}

entity_record ecs_entity_index_get_entity(const entity_index *const index_ref, entity_id id)
{
    if (id < index_ref->capacity)
    {
        return index_ref->data[id];
    }
    return (entity_record){.row = 0, .storage = 0};
}