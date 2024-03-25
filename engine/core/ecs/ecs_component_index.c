#include <stdint.h>

#include "ecs_component_index.h"
#include "../caffeine_memory.h"
#include "ecs_name_index.h"
#include "../caffeine_logging.h"

typedef struct
{
    component_id id;
    const char *name;
    size_t size;
    size_t align;
} component_info;

struct component_index
{
    component_info *data_owning;
    uint32_t count;
    uint32_t capacity;
    name_index name_table;
};

component_index *ecs_new_component_index(uint32_t capacity)
{
    component_index *instance_owning = (component_index *)CFF_ALLOC(sizeof(component_index), "COMPONENT INDEX");

    if (instance_owning == NULL)
    {
        caff_log_error("[COMPONENT INDEX] Creation error: fail to allocate component index memory\n");
        return instance_owning;
    }

    component_info *buffer_owning = (component_info *)CFF_ALLOC(capacity * sizeof(component_info), "COMPONENT INDEX BUFFER");

    if (buffer_owning == NULL)
    {
        caff_log_error("[COMPONENT INDEX] Creation error: fail to allocate buffer memory for component index\n");
        CFF_RELEASE(instance_owning);
        return NULL;
    }

    CFF_ZERO(buffer_owning, capacity * sizeof(component_info));

    instance_owning->capacity = capacity;
    instance_owning->count = 0;
    instance_owning->data_owning = buffer_owning;

    ecs_name_index_init(&(instance_owning->name_table));

    return instance_owning;
}

component_id ecs_register_component(component_index *const index_mut_ref, const char *const name_ref, component_type type, size_t size, size_t align)
{
    name_index *name_table = &(index_mut_ref->name_table);
    if (name_table == NULL)
    {
        caff_log_error("[COMPONENT INDEX] Component registering error: component_index->name_table was null\n");
        return INVALID_ID;
    }

    component_id existent_id = INVALID_ID;

    ecs_name_index_get(name_table, (caff_string)name_ref, &existent_id);

    if (existent_id != INVALID_ID)
    {
        caff_log_trace("[COMPONENT INDEX] Component %s alread exists with id %u\n", name_ref, existent_id);
        return existent_id;
    }

    uint32_t components_count = index_mut_ref->count;
    uint16_t flags = (uint16_t)type;

    component_id_metadata id_meta = {0};
    id_meta.flags |= flags;
    id_meta.index = components_count;
    id_meta.scope = 0;

    component_id id = component_id_pack(id_meta);

    if (index_mut_ref->count == index_mut_ref->capacity)
    {
        index_mut_ref->data_owning = CFF_ARR_RESIZE(index_mut_ref->data_owning, index_mut_ref->capacity * 2);
        index_mut_ref->capacity *= 2;

        if (index_mut_ref->data_owning == NULL)
        {
            caff_log_error("[COMPONENT INDEX] Component registering error: failed to resize component_index->data_owning buffer\n");
            return INVALID_ID;
        }
    }

    component_info info = {
        .id = id,
        .name = name_ref,
        .size = size,
        .align = align,
    };

    index_mut_ref->data_owning[id_meta.index] = info;
    index_mut_ref->count++;

    ecs_name_index_add(name_table, (caff_string)name_ref, id);
    caff_log_trace("[COMPONENT INDEX] Component %s registered with id %" PRIu64 "\n", name_ref, id);
    return id;
}

component_id ecs_get_component_id(const component_index *const index_ref, const char *const name)
{
    caff_string name_str = (caff_string)name;
    const name_index *ni = &(index_ref->name_table);

    if (ni == NULL)
    {
        caff_log_error("[COMPONENT INDEX] Failed to get component id: component_index->name_table was null\n");
        return INVALID_ID;
    }

    component_id existent_id = INVALID_ID;
    ecs_name_index_get(ni, name_str, &existent_id);

    if (existent_id == INVALID_ID)
    {
        caff_log_warn("[COMPONENT INDEX] Failed to get component id: Component %s not found\n", name);
    }
    else
    {
        caff_log_trace("[COMPONENT INDEX] Component %s id found with value %" PRIu64 "\n", name, existent_id);
    }

    return existent_id;
}

size_t ecs_get_component_size(const component_index *const index_ref, component_id id)
{
    if (id == INVALID_ID)
    {
        caff_log_warn("[COMPONENT INDEX] Failed to get component size: invalid id\n");
        return 0;
    }
    component_id_metadata id_meta = component_id_unpack(id);
    uint32_t index = id_meta.index;

    if (index >= index_ref->count)
    {
        caff_log_error("[COMPONENT INDEX] Failed to get component size: invalid id.index\n");
        return 0;
    }

    caff_log_trace("[COMPONENT INDEX] Component %s size found with value %u\n", index_ref->data_owning[index].name, index_ref->data_owning[index].size);
    return index_ref->data_owning[index].size;
}

size_t ecs_get_component_align(const component_index *const index_ref, component_id id)
{
    if (id == INVALID_ID)
    {
        caff_log_warn("[COMPONENT INDEX] Failed to get component align: invalid id\n");
        return 0;
    }
    component_id_metadata id_meta = component_id_unpack(id);
    uint32_t index = id_meta.index;

    if (index >= index_ref->count)
    {
        caff_log_error("[COMPONENT INDEX] Failed to get component align: invalid id.index\n");
        return 0;
    }

    caff_log_trace("[COMPONENT INDEX] Component %s align found with value %u\n", index_ref->data_owning[index].name, index_ref->data_owning[index].align);
    return index_ref->data_owning[index].align;
}

const char *const ecs_get_component_name(const component_index *const index_ref, component_id id)
{
    if (id == INVALID_ID)
    {
        caff_log_warn("[COMPONENT INDEX] Failed to get component name: invalid id\n");
        return NULL;
    }

    component_id_metadata id_meta = component_id_unpack(id);
    uint32_t index = id_meta.index;
    if (index >= index_ref->count)
    {
        caff_log_error("[COMPONENT INDEX] Failed to get component name: invalid id.index\n");
        return NULL;
    }

    caff_log_trace("[COMPONENT INDEX] Component %" PRIu64 " name found with value %s\n", index_ref->data_owning[index].id, index_ref->data_owning[index].name);
    return (const char *const)index_ref->data_owning[index].name;
}

void ecs_remove_component(component_index *const index_mut_ref, component_id id)
{
    if (id == INVALID_ID)
    {
        caff_log_warn("[COMPONENT INDEX] Failed to remove component: invalid id\n");
        return;
    }

    component_id_metadata id_meta = component_id_unpack(id);
    uint32_t index = id_meta.index;

    if (index >= index_mut_ref->count)
    {
        caff_log_error("[COMPONENT INDEX] Failed to get component name: invalid id.index\n");
        return;
    }

    caff_string name = (caff_string)index_mut_ref->data_owning[index].name;

    ecs_name_index_remove(&(index_mut_ref->name_table), name);

    index_mut_ref->data_owning[index] = (component_info){.id = 0, .name = 0, .size = 0, .align = 0};
    caff_log_trace("[COMPONENT INDEX] Component %s removed\n", name);
}

void ecs_release_component_index(const component_index *const index_owning)
{
    const name_index *ni = &(index_owning->name_table);
    if (ni == NULL)
    {
        caff_log_error("[COMPONENT INDEX] Failed to release component index: component index -> name index was null\n");
        return;
    }
    else
    {
        ecs_name_index_release(ni);
        caff_log_trace("[COMPONENT INDEX] Component index -> name index released\n");
    }
    CFF_RELEASE(index_owning->data_owning);
    CFF_RELEASE(index_owning);

    caff_log_trace("[COMPONENT INDEX] Component index released\n");
}