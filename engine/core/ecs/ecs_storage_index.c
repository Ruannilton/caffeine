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

ecs_storage ecs_storage_new(const component_id *const components, const size_t *const component_sizes, const char **const names_owning, uint32_t components_count);
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

void ecs_storage_index_new_storage(
    storage_index *const index_mut_ref,
    archetype_id arch_id,
    const component_id *const components_owning,
    const size_t *const sizes_owning,
    const char **const names_owning,
    uint32_t lenght)
{
    if (arch_id > index_mut_ref->capacity)
    {
        uint32_t new_capacity = index_mut_ref->capacity * 2;

        while (arch_id > new_capacity)
        {
            new_capacity *= 2;
        }

        index_mut_ref->storages = CFF_ARR_RESIZE(index_mut_ref->storages, new_capacity);
        index_mut_ref->used = CFF_ARR_RESIZE(index_mut_ref->used, new_capacity);
        index_mut_ref->capacity = new_capacity;
    }

    if (index_mut_ref->count == index_mut_ref->capacity)
    {
        uint32_t new_capacity = index_mut_ref->capacity * 2;
        index_mut_ref->storages = CFF_ARR_RESIZE(index_mut_ref->storages, new_capacity);
        index_mut_ref->used = CFF_ARR_RESIZE(index_mut_ref->used, new_capacity);
        index_mut_ref->capacity = new_capacity;
    }

    index_mut_ref->storages[arch_id] = ecs_storage_new(components_owning, sizes_owning, names_owning, lenght);
    index_mut_ref->used[arch_id] = 1;
    index_mut_ref->count++;
}

ecs_storage *ecs_storage_index_get(const storage_index *const index_ref, archetype_id arch_id)
{
    if (index_ref->used[arch_id])
        return (ecs_storage *)(&index_ref->storages[arch_id]);
    return NULL;
}
void ecs_storage_index_remove(storage_index *const index_mut_ref, archetype_id arch_id)
{
    index_mut_ref->used[arch_id] = 0;
    ecs_storage_release(index_mut_ref->storages + arch_id);
    index_mut_ref->count--;
}