#include "caffeine_2_ecs.h"

#define COMPONENT_TABLE_DEFAULT_SIZE 8
#define ARCH_HASH_TABLE_DEFAULT_SIZE 256
#define STORAGE_TABLE_DEFAULT_CAPACITY 4

typedef struct
{
    component_id id;
    uint32_t size;
    const char *name;
} component_metadata;

typedef struct
{
    entity_id ent_id;
    archtype_id arch_id;
    uint32_t instance;
} entity_metadata;

typedef struct
{
    uint32_t entity_size;
    archtype_id arch_id;
    component_id *comp_ids;
    uint32_t *comp_sizes;
    uint32_t component_count;
    void **component_datas;
    entity_id *entity_ids;
    uint32_t count, capacity;
} entity_storage;

typedef struct
{
    archtype_id id;
    archetype arch;
    entity_storage *storage;
} archetype_metadata;

typedef struct
{
    component_id *comp_ids;
    uint32_t component_count;
    archtype_id id;
} kv_components_id;

typedef struct
{
    entity_id id;
    archtype_id arch_id;
    uint32_t row;
} entity_record;

typedef struct
{
    uint32_t row;
    entity_storage *storage;
} entity_ref;

cff_arr_dcltype(component_table, component_metadata);
cff_arr_dcltype(archetype_table, archetype_metadata);
cff_arr_dcltype(storage_table, entity_storage);
cff_arr_dcltype(arch_to_id, kv_components_id);
cff_arr_dcltype(entity_table, entity_record);

static component_table components;
static archetype_table archetypes;
static arch_to_id arch_hash;
static storage_table storages;
static uint32_t arch_hash_colision = 0;
static entity_table entities;

static uint32_t hash_archetype(uint32_t len, component_id components[len], uint32_t seed);
static inline entity_ref ecs_get_entity(entity_id id);
static inline void *ecs_get_storage_component(entity_storage *storage, uint32_t row, component_id component);
static void entity_storage_release(entity_storage *storage);
static void entity_storage_init(entity_storage *storage, archtype_id arch_id, archetype arch);
static int entity_storage_allocate(entity_storage *storage, entity_id id);
static bool archetype_compare(uint32_t a_count, uint32_t b_count, component_id a[a_count], component_id b[b_count]);

bool ecs_init()
{
    cff_arr_init(&components, COMPONENT_TABLE_DEFAULT_SIZE);
    cff_arr_init(&archetypes, ARCH_HASH_TABLE_DEFAULT_SIZE);
    cff_arr_init(&arch_hash, ARCH_HASH_TABLE_DEFAULT_SIZE);
    cff_arr_init(&storages, ARCH_HASH_TABLE_DEFAULT_SIZE);
    cff_arr_init(&entities, STORAGE_TABLE_DEFAULT_CAPACITY);

    // dummy component
    component_metadata dummy_component = {.id = 0, .name = "__Invalid", .size = 0};
    cff_arr_add(&components, dummy_component);

    // dummy archetype
    archetype_metadata dummy_archetype_meta = {0};
    cff_arr_add(&archetypes, dummy_archetype_meta);

    // dummy entity
    cff_arr_add(&entities, (entity_record){0});

    // zero hash_map
    for (size_t i = 0; i < arch_hash.capacity; i++)
    {
        arch_hash.buffer[i] = (kv_components_id){0};
    }

    return true;
}

bool ecs_end()
{
    cff_arr_release(&components);

    // release archetypes
    for (size_t i = 0; i < archetypes.count; i++)
    {
        cff_mem_release(archetypes.buffer[i].arch.buffer); // release archetypes buffers
    }
    cff_arr_release(&archetypes);

    cff_arr_release(&arch_hash);

    // release storages
    for (size_t i = 0; i < storages.count; i++)
    {
        entity_storage_release(storages.buffer + i);
    }
    cff_arr_release(&storages);

    cff_arr_release(&entities);

    return true;
}

component_id ecs_register_component(const char *name, uint32_t size)
{
    component_id new_component = components.count;

    component_metadata metadata = {.id = new_component, .name = name, .size = size};

    uint32_t inserted_at;
    cff_arr_add_i(&components, metadata, inserted_at);

    debug_assert(inserted_at == (uint32_t)metadata.id);

    return new_component;
}

archtype_id ecs_register_archetype(archetype arch)
{
    uint32_t colision = 0;
    uint32_t hash = hash_archetype(arch.count, arch.buffer, colision) % arch_hash.capacity;

    while (arch_hash.buffer[hash].id != INVALID_ID)
    {
        if (archetype_compare(arch_hash.buffer[hash].component_count, arch.count, arch_hash.buffer[hash].comp_ids, arch.buffer))
            return arch_hash.buffer[hash].id;

        colision++;
        hash = hash_archetype(arch.count, arch.buffer, colision) % arch_hash.capacity;
    }

    if (colision > arch_hash_colision)
        arch_hash_colision = colision;

    archtype_id arch_id = archetypes.count;

    cff_arr_add_at(&storages, (entity_storage){0}, arch_id);
    entity_storage *storage = &(cff_arr_get(&storages, arch_id));
    entity_storage_init(storage, arch_id, arch);

    archetype_metadata metadata = {.id = arch_id, .arch = arch, .storage = storage};
    cff_arr_add_at(&archetypes, metadata, metadata.id);

    kv_components_id *hash_insert = &(cff_arr_get(&arch_hash, hash));
    hash_insert->id = arch_id;
    hash_insert->component_count = arch.count;
    hash_insert->comp_ids = arch.buffer;
    arch_hash.count++;

    if (arch_hash.count > arch_hash.capacity * 0.7f)
    {
        cff_arr_resize(&arch_hash, arch_hash.capacity * 2);
        // rehash
    }

    return arch_id;
}

archtype_id ecs_archetype_get(uint32_t len, component_id components[len])
{
    uint32_t colision = 0;
    uint32_t hash = hash_archetype(len, components, colision) % arch_hash.capacity;

    while (arch_hash.buffer[hash].id != INVALID_ID)
    {
        if (archetype_compare(arch_hash.buffer[hash].component_count, len, arch_hash.buffer[hash].comp_ids, components))
            return arch_hash.buffer[hash].id;

        colision++;

        if (colision > arch_hash_colision)
            return INVALID_ID;
        hash = hash_archetype(len, components, colision) % arch_hash.capacity;
    }

    return INVALID_ID;
}

entity_id ecs_create_entity(archtype_id archetype)
{
    entity_id id = (entity_id)entities.count;
    entity_storage *storage = &(cff_arr_get(&storages, archetype));
    int row = entity_storage_allocate(storage, id);
    entity_record record = {.arch_id = archetype, .id = id, .row = row};

    cff_arr_add_at(&entities, record, id);

    return id;
}

void *ecs_get_entity_component(entity_id e_id, component_id c_id)
{
    entity_ref entity = ecs_get_entity(e_id);
    void *component_ref = ecs_get_storage_component(entity.storage, entity.row, c_id);
    return component_ref;
}

void ecs_destroy_entity(entity_id id)
{
    entity_record *entity_data = &(cff_arr_get(&entities, id));
    archtype_id arch_id = entity_data->arch_id;
    entity_storage *entity_storage = &(cff_arr_get(&storages, arch_id));

    uint32_t entity_index_row = entity_data->row;

    uint32_t last_entity_row = entity_storage->count - 1;
    entity_id last_entity_id = entity_storage->entity_ids[last_entity_row];
    entity_record *last_entity_data = &(cff_arr_get(&entities, last_entity_id));

    uint8_t swap_buffer[entity_storage->entity_size];

    for (size_t i = 0; i < entity_storage->component_count; i++)
    {
        uint32_t component_size = entity_storage->comp_sizes[i];

        void *last_component = (void *)((uintptr_t)entity_storage->component_datas[i] + (uintptr_t)((component_size)*last_entity_row));
        void *entity_component = (void *)((uintptr_t)entity_storage->component_datas[i] + (uintptr_t)((component_size)*entity_index_row));

        cff_mem_copy(last_component, (void *)swap_buffer, component_size);
        cff_mem_copy(entity_component, last_component, component_size);
        cff_mem_copy((void *)swap_buffer, entity_component, component_size);
    }

    entity_id tmp = entity_storage->entity_ids[last_entity_row];
    entity_storage->entity_ids[last_entity_row] = id;
    entity_storage->entity_ids[entity_index_row] = tmp;

    entity_data->row = last_entity_row;
    last_entity_data->row = entity_index_row;
}

//-------------------

static inline entity_ref ecs_get_entity(entity_id id)
{
    entity_record *entity_data = &(cff_arr_get(&entities, id));
    archtype_id arch_id = entity_data->arch_id;
    entity_storage *entity_storage = &(cff_arr_get(&storages, arch_id));

    uint32_t entity_index_row = entity_data->row;

    return (entity_ref){.row = entity_index_row, .storage = entity_storage};
}

static uint32_t hash_archetype(uint32_t len, component_id components[len], uint32_t seed)
{
    uint32_t hash_value = seed;

    for (size_t i = 0; i < len; i++)
    {
        // Mix the bits of the current integer into the hash value
        hash_value ^= components[i];
        hash_value *= 0x9e3779b1; // A prime number for mixing
    }

    return hash_value;
}

static bool archetype_compare(uint32_t a_count, uint32_t b_count, component_id a[a_count], component_id b[b_count])
{
    if (a_count != b_count)
        return false;
    for (size_t i = 0; i < a_count; i++)
    {
        if (a[i] != b[i])
            return false;
    }
    return true;
}

static void entity_storage_init(entity_storage *storage, archtype_id arch_id, archetype arch)
{
    uint32_t component_count = arch.count;
    storage->entity_size = 0;
    storage->arch_id = arch_id;
    storage->component_count = component_count;
    storage->comp_ids = arch.buffer;

    storage->count = 0;
    storage->capacity = STORAGE_TABLE_DEFAULT_CAPACITY;

    storage->comp_sizes = cff_mem_alloc(sizeof(uint32_t) * component_count);
    storage->component_datas = cff_mem_alloc(sizeof(void *) * component_count);

    storage->entity_ids = cff_mem_alloc(sizeof(entity_id) * storage->capacity);
    cff_mem_zero(storage->entity_ids, sizeof(entity_id), sizeof(entity_id) * storage->capacity);

    for (size_t i = 0; i < component_count; i++)
    {
        component_id comp_id = arch.buffer[i];
        uint32_t comp_size = components.buffer[comp_id].size;

        storage->comp_sizes[i] = comp_size;
        storage->entity_size += comp_size;
        void *component_buffer = cff_mem_alloc(comp_size * storage->capacity);
        cff_mem_zero(component_buffer, comp_size, comp_size * storage->capacity);
        storage->component_datas[i] = component_buffer;
    }
}

static void entity_storage_release(entity_storage *storage)
{
    for (size_t i = 0; i < storage->component_count; i++)
    {
        cff_mem_release(storage->component_datas[i]);
    }
    cff_mem_release(storage->entity_ids);
    cff_mem_release(storage->comp_sizes);
    cff_mem_release(storage->component_datas);
}

static int entity_storage_allocate(entity_storage *storage, entity_id id)
{
    if (storage->count == storage->capacity)
    {
        storage->capacity *= 2;

        for (size_t i = 0; i < storage->component_count; i++)
        {
            uint32_t size = storage->comp_sizes[i];
            void *component_buffer = storage->component_datas[i];
            storage->component_datas[i] = cff_mem_realloc(component_buffer, size * storage->capacity);
        }

        storage->entity_ids = cff_mem_realloc(storage->entity_ids, sizeof(entity_id) * storage->capacity);
    }

    int index = storage->count;

    storage->entity_ids[index] = id;

    storage->count++;

    return index;
}

static inline void *ecs_get_storage_component(entity_storage *storage, uint32_t row, component_id component)
{
    for (size_t i = 0; i < storage->component_count; i++)
    {
        if (storage->comp_ids[i] != component)
            continue;

        uint32_t component_size = storage->comp_sizes[i];

        void *entity_component = (void *)((uintptr_t)storage->component_datas[i] + (uintptr_t)((component_size)*row));

        return entity_component;
    }

    return NULL;
}