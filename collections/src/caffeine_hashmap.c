#include "caffeine_hashmap.h"

static uint64_t cff_hashmp_get_capacity(cff_hashmap_s *container)
{
    return container->capacity;
}

static uint64_t cff_hashmp_get_count(cff_hashmap_s *container)
{
    return container->count;
}

static cff_size cff_hashmp_get_data_size(cff_hashmap_s *container)
{
    return container->data_size;
}

static bool cff_hashmp_get_item_ref(cff_hashmap_s *container, uint64_t index, uintptr_t *out)
{
    if (!container->used_slot[index])
        return false;

    *out = (uintptr_t)container->keys + (uintptr_t)(container->key_size * index);
    return true;
}

static cff_hashmap_s _hashmap_resize(cff_hashmap_s hashmap, uint64_t new_size, cff_allocator_t allocator)
{
    uintptr_t data_buffer = (uintptr_t)(cff_allocator_allocate(&allocator, (cff_size)(new_size * hashmap.data_size)));

    if (data_buffer == 0)
        return (cff_hashmap_s){.error_code = CFF_ERR_ALLOC};

    uintptr_t key_buffer = (uintptr_t)(cff_allocator_allocate(&allocator, (cff_size)(new_size * hashmap.key_size)));

    if (key_buffer == 0)
    {
        cff_allocator_release(&allocator, (void *)data_buffer);
        return (cff_hashmap_s){.error_code = CFF_ERR_ALLOC};
    }

    bool *used_buffer = (bool *)(cff_allocator_allocate(&allocator, (cff_size)(new_size * sizeof(bool))));

    if (used_buffer == 0)
    {
        cff_allocator_release(&allocator, (void *)data_buffer);
        cff_allocator_release(&allocator, (void *)key_buffer);
        return (cff_hashmap_s){.error_code = CFF_ERR_ALLOC};
    }

    uint64_t max_collisions = 0;

    for (uint64_t i = 0; i < hashmap.capacity; i++)
    {
        if (!hashmap.used_slot[i])
            continue;

        uintptr_t key = (uintptr_t)hashmap.keys * (uintptr_t)(hashmap.key_size * i);
        uintptr_t value = (uintptr_t)hashmap.data * (uintptr_t)(hashmap.data_size * i);

        uint64_t collision_count = 0;
        uint64_t entry_index = hashmap.hash_fn(key, hashmap.key_size, 0) % new_size;

        while (used_buffer[entry_index])
        {
            collision_count++;
            entry_index = hashmap.hash_fn(key, hashmap.key_size, collision_count) % new_size;
        }

        if (max_collisions < collision_count)
            max_collisions = collision_count;

        used_buffer[entry_index] = true;
        uintptr_t current_data_slot = ((uintptr_t)data_buffer) + (uintptr_t)(hashmap.data_size * entry_index);
        cff_mem_copy((const void *)value, (void *)current_data_slot, hashmap.data_size);

        uintptr_t current_key_slot = ((uintptr_t)key_buffer) + (uintptr_t)(hashmap.key_size * entry_index);
        cff_mem_copy((const void *)value, (void *)current_key_slot, hashmap.key_size);
    }

    cff_allocator_release(&allocator, hashmap.data);
    cff_allocator_release(&allocator, hashmap.keys);
    cff_allocator_release(&allocator, hashmap.used_slot);

    hashmap.collision_count = max_collisions;
    hashmap.data = (void *)data_buffer;
    hashmap.keys = (void *)key_buffer;
    hashmap.used_slot = used_buffer;

    return hashmap;
}

cff_hashmap_s cff_hashmap_create(cff_size key_size, cff_size data_size, uint64_t capacity, cff_comparer_function key_cmp_fn, cff_comparer_function data_cmp_fn, cff_hash_function hash_fn, cff_allocator_t allocator)
{

    uintptr_t data_buffer = (uintptr_t)(cff_allocator_allocate(&allocator, (cff_size)(capacity * data_size)));
    if (data_buffer == 0)
    {
        return (cff_hashmap_s){.error_code = CFF_ERR_ALLOC};
    }

    uintptr_t key_buffer = (uintptr_t)(cff_allocator_allocate(&allocator, (cff_size)(capacity * key_size)));

    if (key_buffer == 0)
    {
        cff_allocator_release(&allocator, (void *)data_buffer);
        return (cff_hashmap_s){.error_code = CFF_ERR_ALLOC};
    }

    bool *used_buffer = (bool *)(cff_allocator_allocate(&allocator, (cff_size)(capacity * sizeof(bool))));

    if (used_buffer == 0)
    {
        cff_allocator_release(&allocator, (void *)data_buffer);
        cff_allocator_release(&allocator, (void *)key_buffer);
        return (cff_hashmap_s){.error_code = CFF_ERR_ALLOC};
    }

    bool cnt = false;
    cff_mem_set((const void *)&cnt, (void *)used_buffer, (cff_size)sizeof(bool), (cff_size)(capacity * sizeof(bool)));

    return (cff_hashmap_s){
        .capacity = capacity,
        .data = (void *)data_buffer,
        .keys = (void *)key_buffer,
        .used_slot = used_buffer,
        .hash_fn = hash_fn,
        .key_cmp_fn = key_cmp_fn,
        .data_cmp_fn = data_cmp_fn,
    };
}

bool cff_hashmap_add(cff_hashmap_s hashmap, uintptr_t key, uintptr_t value, cff_allocator_t allocator)
{
    if ((float)hashmap.count / hashmap.capacity >= 0.75f)
    {
        hashmap = _hashmap_resize(hashmap, hashmap.capacity * 2, allocator);
        if (hashmap.error_code != CFF_ERR_NONE)
            return false;
    }

    uint64_t collision_count = 0;
    uint64_t entry_index = hashmap.hash_fn(key, hashmap.key_size, 0) % hashmap.capacity;

    while (hashmap.used_slot[entry_index])
    {
        uintptr_t current_key = (uintptr_t)hashmap.keys + (uintptr_t)(collision_count * hashmap.key_size);

        if (hashmap.key_cmp_fn(key, current_key, hashmap.key_size))
            continue;

        collision_count++;
        entry_index = hashmap.hash_fn(key, hashmap.key_size, collision_count) % hashmap.capacity;
    }

    if (hashmap.collision_count < collision_count)
        hashmap.collision_count = collision_count;

    hashmap.used_slot[entry_index] = true;
    uintptr_t current_data_slot = ((uintptr_t)hashmap.data) + (uintptr_t)(hashmap.data_size * entry_index);
    cff_mem_copy((const void *)value, (void *)current_data_slot, hashmap.data_size);

    uintptr_t current_key_slot = ((uintptr_t)hashmap.keys) + (uintptr_t)(hashmap.key_size * entry_index);
    cff_mem_copy((const void *)value, (void *)current_key_slot, hashmap.key_size);

    hashmap.count++;
    return true;
}

bool cff_hashmap_get(cff_hashmap_s hashmap, uintptr_t key, uintptr_t *value)
{
    uint64_t collision_count = 0;
    uint64_t entry_index = hashmap.hash_fn(key, hashmap.key_size, 0) % hashmap.capacity;
    uintptr_t current_key = (uintptr_t)hashmap.keys + (uintptr_t)(collision_count * hashmap.key_size);

    while (!hashmap.used_slot[entry_index] && !hashmap.key_cmp_fn(key, current_key, hashmap.key_size))
    {
        current_key = (uintptr_t)hashmap.keys + (uintptr_t)(collision_count * hashmap.key_size);
        collision_count++;

        if (collision_count > hashmap.collision_count)
            return false;

        entry_index = hashmap.hash_fn(key, hashmap.key_size, collision_count) % hashmap.capacity;
    }

    if (value != NULL)
    {
        *value = ((uintptr_t)hashmap.data) + (uintptr_t)(hashmap.data_size * entry_index);
        return true;
    }

    return false;
}

bool cff_hashmap_remove(cff_hashmap_s hashmap, uintptr_t key, cff_allocator_t allocator)
{
    uint64_t collision_count = 0;
    uint64_t entry_index = hashmap.hash_fn(key, hashmap.key_size, 0) % hashmap.capacity;
    uintptr_t current_key = ((uintptr_t)hashmap.keys) + (uintptr_t)(collision_count * hashmap.key_size);

    while (!hashmap.used_slot[entry_index] && !hashmap.key_cmp_fn(key, current_key, hashmap.key_size))
    {
        current_key = ((uintptr_t)hashmap.keys) + (uintptr_t)(collision_count * hashmap.key_size);
        collision_count++;

        if (collision_count > hashmap.collision_count)
            return false;

        entry_index = hashmap.hash_fn(key, hashmap.key_size, collision_count) % hashmap.capacity;
    }

    hashmap.used_slot[entry_index] = false;
    hashmap.count--;

    if ((float)hashmap.count / hashmap.capacity < 0.25f)
    {
        hashmap = _hashmap_resize(hashmap, hashmap.capacity / 2, allocator);
        if (hashmap.error_code != CFF_ERR_NONE)
            return false;
    }

    return true;
}

bool cff_hashmap_contains_key(cff_hashmap_s hashmap, uintptr_t key)
{
    uint64_t collision_count = 0;
    uint64_t entry_index = hashmap.hash_fn(key, hashmap.key_size, 0) % hashmap.capacity;
    uintptr_t current_key = ((uintptr_t)hashmap.keys) + (uintptr_t)(collision_count * hashmap.key_size);

    while (!hashmap.used_slot[entry_index] && !hashmap.key_cmp_fn(key, current_key, hashmap.key_size))
    {
        current_key = ((uintptr_t)hashmap.keys) + (uintptr_t)(collision_count * hashmap.key_size);
        collision_count++;

        if (collision_count > hashmap.collision_count)
            return false;

        entry_index = hashmap.hash_fn(key, hashmap.key_size, collision_count) % hashmap.capacity;
    }

    return true;
}

bool cff_hashmap_contains_value(cff_hashmap_s hashmap, uintptr_t value)
{
    for (uint64_t i = 0; i < hashmap.capacity; i++)
    {
        if (!hashmap.used_slot[i])
            continue;

        uintptr_t ptr = ((uintptr_t)hashmap.data) + (uintptr_t)(hashmap.data_size * i);

        if (hashmap.data_cmp_fn(value, ptr, hashmap.data_size))
            return true;
    }

    return false;
}

void cff_hashmap_clear(cff_hashmap_s hashmap)
{
    hashmap.collision_count = 0;
    bool cnt = false;
    cff_mem_set(&cnt, hashmap.used_slot, (cff_size)sizeof(bool), (cff_size)(hashmap.capacity * sizeof(bool)));
}

void cff_hashmap_destroy(cff_hashmap_s hashmap, cff_allocator_t allocator)
{
    cff_allocator_release(&allocator, hashmap.data);
    cff_allocator_release(&allocator, hashmap.keys);
    cff_allocator_release(&allocator, hashmap.used_slot);

    hashmap = (cff_hashmap_s){0};
}

cff_hashmap_s cff_hashmap_copy(cff_hashmap_s hashmap, cff_allocator_t allocator)
{
    uint64_t new_size = hashmap.count * 2;

    cff_hashmap_s new_hash = cff_hashmap_create(hashmap.key_size, hashmap.data_size, new_size, hashmap.key_cmp_fn, hashmap.data_cmp_fn, hashmap.hash_fn, allocator);

    cff_iterator_s it = cff_hashmap_get_iterator(&hashmap);

    while (it.current_item)
    {
        uintptr_t key = cff_iterator_current(&it);
        uintptr_t value;
        cff_hashmap_get(hashmap, key, &value);
        cff_hashmap_add(new_hash, key, value, allocator);
        cff_iterator_next(&it);
    }
    return new_hash;
}

cff_hashmap_s cff_hashmap_clone(cff_hashmap_s hashmap, cff_allocator_t allocator)
{
    cff_hashmap_s new_hash = cff_hashmap_create(hashmap.key_size, hashmap.data_size, hashmap.capacity, hashmap.key_cmp_fn, hashmap.data_cmp_fn, hashmap.hash_fn, allocator);

    new_hash.collision_count = hashmap.collision_count;
    new_hash.count = hashmap.count;

    cff_mem_copy(hashmap.data, new_hash.data, (cff_size)(hashmap.data_size * hashmap.capacity));
    cff_mem_copy(hashmap.keys, new_hash.keys, (cff_size)(hashmap.key_size * hashmap.capacity));
    cff_mem_copy(hashmap.used_slot, new_hash.used_slot, (cff_size)(sizeof(bool) * hashmap.capacity));

    return new_hash;
}

cff_iterator_s cff_hashmap_get_iterator(cff_hashmap_s *hashmap)
{
    cff_iterator_s it = {
        .index = 0,
        .current_item = 0,
        .data = hashmap,
        .interf = (cff_container_iterator_i){
            .get_capacity = cff_hashmp_get_capacity,
            .get_count = cff_hashmp_get_capacity,
            .get_data_size = cff_hashmp_get_data_size,
            .get_item_ref = cff_hashmp_get_item_ref,
        },
    };

    cff_iterator_reset(&it);

    return it;
}