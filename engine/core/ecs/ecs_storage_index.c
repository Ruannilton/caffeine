#include "ecs_storage_index.h"
#include "../caffeine_memory.h"

struct storage_index
{
    uint32_t capacity;
    uint32_t count;
    ecs_storage **storages;
};

void ecs_storage_release(ecs_storage *storage);

storage_index *ecs_storage_index_new(uint32_t capacity)
{
    storage_index *index = cff_mem_alloc(sizeof(storage_index));

    index->storages = (ecs_storage **)cff_mem_alloc(sizeof(ecs_storage *) * capacity);
    index->capacity = capacity;
    index->count = 0;

    cff_mem_zero(index->storages, sizeof(ecs_storage *), sizeof(ecs_storage *) * capacity);

    return index;
}

void ecs_storage_index_release(storage_index *index)
{
    for (size_t i = 0; i < index->capacity; i++)
    {
        if (index->storages[i] != NULL)
        {
            ecs_storage_release(index->storages[i]);
        }
    }

    cff_mem_release(index->storages);
    cff_mem_release(index);
}

void ecs_storage_index_set(storage_index *index, archetype_id arch_id, ecs_storage *storage)
{
    if (arch_id > index->capacity)
    {
        cff_resize_arr(index->storages, arch_id + 1);
    }

    index->storages[arch_id] = storage;
    index->count++;
}
ecs_storage *ecs_storage_index_get(storage_index *index, archetype_id arch_id)
{
    return index->storages[arch_id];
}
void ecs_storage_index_remove(storage_index *index, archetype_id arch_id)
{
    index->storages[arch_id] = NULL;
    index->count--;
}