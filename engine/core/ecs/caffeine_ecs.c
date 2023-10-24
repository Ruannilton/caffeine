#include "caffeine_ecs.h"

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
    entity_id id;
    archtype_id arch_id;
    uint32_t row;
} entity_record;

typedef struct
{
    uint32_t row;
    entity_storage *storage;
} entity_ref;

cff_arr_dcltype(component_list, component_metadata);
cff_arr_impl(component_list, component_metadata);

// List<component_metadata>
static component_list components;

cff_arr_dcltype(archetype_list, archetype_metadata);
cff_arr_impl(archetype_list, archetype_metadata);

// List<archetype_metadata>
static archetype_list archetypes;

cff_arr_dcltype(entity_list, entity_record);
cff_arr_impl(entity_list, entity_record);

// List<entity_record>
static entity_list entities;

cff_arr_dcltype(storage_list, entity_storage);
cff_arr_impl(storage_list, entity_storage);

// List<entity_storage>
static storage_list storages;

cff_arr_dcltype(arch_meta_list, archetype_metadata *);
cff_arr_impl(arch_meta_list, archetype_metadata *);
cff_arr_dcltype(component_usage_list, arch_meta_list);
cff_arr_impl(component_usage_list, arch_meta_list);

// List<List<archetype_metadata*>>
static component_usage_list component_reference_count;

cff_hash_dcltype(archetype_table, archetype, archtype_id);
cff_hash_impl(archetype_table, archetype, archtype_id);

// Dictionary<archetype,archtype_id>
static archetype_table arch_table;

static uint32_t hash_archetype(archetype *data, uint32_t seed);
static bool compare_archetype(archetype *a, archetype *b);
static bool compare_archetype_id(archtype_id *a, archtype_id *b);

static inline entity_ref ecs_get_entity(entity_id id);
static inline void *ecs_get_storage_component(entity_storage *storage, uint32_t row, component_id component);
static void entity_storage_release(entity_storage *storage);
static void entity_storage_init(entity_storage *storage, archtype_id arch_id, archetype arch);
static int entity_storage_allocate(entity_storage *storage, entity_id id);

bool ecs_init()
{
    component_list_init(&components, COMPONENT_TABLE_DEFAULT_SIZE);
    component_usage_list_init(&component_reference_count, COMPONENT_TABLE_DEFAULT_SIZE);
    archetype_list_init(&archetypes, ARCH_HASH_TABLE_DEFAULT_SIZE);
    storage_list_init(&storages, ARCH_HASH_TABLE_DEFAULT_SIZE);
    entity_list_init(&entities, STORAGE_TABLE_DEFAULT_CAPACITY);

    archetype_table_init(&arch_table, ARCH_HASH_TABLE_DEFAULT_SIZE, hash_archetype, compare_archetype, compare_archetype_id);

    // dummy component
    component_metadata dummy_component = {.id = 0, .name = "__Invalid", .size = 0};
    component_list_add(&components, dummy_component);

    // dummy archetype
    archetype_metadata dummy_archetype_meta = {0};
    archetype_list_add(&archetypes, dummy_archetype_meta);

    // dummy entity
    entity_list_add(&entities, (entity_record){0});

    component_usage_list_zero(&component_reference_count);
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

    component_list_add_at(&components, metadata, new_component);

    arch_meta_list usage = {0};
    arch_meta_list_init(&usage, 4);

    component_usage_list_add_at(&component_reference_count, usage, new_component);

    return new_component;
}

archtype_id ecs_register_archetype(archetype arch)
{

    archtype_id new_id = INVALID_ID;
    if (archetype_table_get(&arch_table, arch, &new_id))
    {
        return new_id;
    }

    archtype_id arch_id = arch_table.count;
    archetype_table_add(&arch_table, arch, new_id);

    entity_storage storage = {0};
    entity_storage_init(&storage, arch_id, arch);
    storage_list_add_at(&storages, storage, arch_id);

    entity_storage *storage_ref = storage_list_get_ref(&storages, arch_id);
    archetype_metadata metadata = {.id = arch_id, .arch = arch, .storage = storage_ref};
    archetype_list_add_at(&archetypes, metadata, arch_id);

    archetype_metadata *metadata_ref = archetype_list_get_ref(&archetypes, arch_id);

    for (int i = 0; i < arch.count; i++)
    {
        component_id c_id = arch.buffer[i];
        arch_meta_list *c_usage = component_usage_list_get_ref(&component_reference_count, c_id);

        arch_meta_list_add(c_usage, metadata_ref);
    }

    return arch_id;
}

archtype_id ecs_archetype_get(uint32_t len, component_id components[len])
{
    archtype_id id = INVALID_ID;
    archetype arch = (archetype){.buffer = components, .capacity = len, .count = len};

    int8_t res = archetype_table_get(&arch_table, arch, &id);
    (void)res;

    return id;
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

static uint32_t hash_archetype(archetype *data, uint32_t seed)
{
    uint32_t hash_value = seed;

    for (size_t i = 0; i < data->count; i++)
    {
        // Mix the bits of the current integer into the hash value
        hash_value ^= data->buffer[i];
        hash_value *= 0x9e3779b1; // A prime number for mixing
    }

    return hash_value;
}

static bool compare_archetype(archetype *a, archetype *b)
{
    if (a->count != b->count)
        return false;
    for (size_t i = 0; i < a->count; i++)
    {
        if (a->buffer[i] != b->buffer[i])
            return false;
    }
    return true;
}

static bool compare_archetype_id(archtype_id *a, archtype_id *b)
{
    return *a == *b;
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