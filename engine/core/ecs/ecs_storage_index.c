#include "ecs_storage_index.h"
#include "../caffeine_memory.h"

#include "ecs_storage_type.h"

struct storage_index
{
    uint32_t capacity;
    uint32_t count;
    uint8_t *used;
    ecs_storage *storages;
};

ecs_storage ecs_storage_new(component_id *components, size_t *component_sizes, uint32_t components_count);
void ecs_storage_release(const ecs_storage *const storage);

storage_index *ecs_storage_index_new(uint32_t capacity)
{
    storage_index *index = (storage_index *)CFF_ALLOC(sizeof(storage_index), "STORAGE INDEX");

    index->storages = (ecs_storage *)CFF_ALLOC(sizeof(ecs_storage) * capacity, "STORAGE INDEX BUFFER");
    index->used = (uint8_t *)CFF_ALLOC(sizeof(uint8_t) * capacity, "STORAGE INDEX USED BUFFER");
    index->capacity = capacity;
    index->count = 0;

    CFF_ZERO(index->storages, sizeof(ecs_storage) * capacity);
    CFF_ZERO(index->used, sizeof(uint8_t) * capacity);

    return index;
}

void ecs_storage_index_release(const storage_index *const index_owning)
{
    for (size_t i = 0; i < index_owning->capacity; i++)
    {
        if (index_owning->used[i] != 0)
        {
            ecs_storage_release(index_owning->storages + i);
            index_owning->used[i] = 0;
        }
    }

    CFF_RELEASE(index_owning->storages);
    CFF_RELEASE(index_owning->used);
    CFF_RELEASE(index_owning);
}

void ecs_storage_index_new_storage(storage_index *const index, archetype_id arch_id, component_id *components, size_t *sizes, uint32_t lenght)
{
    if (arch_id > index->capacity)
    {
        uint32_t new_capacity = index->capacity * 2;

        while (arch_id > new_capacity)
        {
            new_capacity *= 2;
        }

        index->storages = CFF_ARR_RESIZE(index->storages, new_capacity);
        index->used = CFF_ARR_RESIZE(index->used, new_capacity);
        index->capacity = new_capacity;
    }

    if (index->count == index->capacity)
    {
        uint32_t new_capacity = index->capacity * 2;
        index->storages = CFF_ARR_RESIZE(index->storages, new_capacity);
        index->used = CFF_ARR_RESIZE(index->used, new_capacity);
        index->capacity = new_capacity;
    }

    index->storages[arch_id] = ecs_storage_new(components, sizes, lenght);
    index->used[arch_id] = 1;
    index->count++;
}

ecs_storage *ecs_storage_index_get(const storage_index *const index, archetype_id arch_id)
{
    if (index->used[arch_id])
        return (ecs_storage *)(&index->storages[arch_id]);
    return NULL;
}
void ecs_storage_index_remove(storage_index *const index, archetype_id arch_id)
{
    index->used[arch_id] = 0;
    ecs_storage_release(index->storages + arch_id);
    index->count--;
}