#pragma once

#include "archetype.h"

typedef struct
{
    struct
    {
        archetype_id arch_id;
        uint32_t component_count;
        component_id *components_ids;
        uint32_t *component_sizes;
    } storage_header;

    void **component_datas;
    void *move_buffer;

    uint32_t *entity_entry;
    uint32_t *entity_entry_reverse;

    uint32_t entity_count;
    uint32_t entity_entry_count;
    uint32_t entity_capacity;

} archetype_storage;

bool ecs_storage_init(archetype_storage *storage, archetype_id id, archetype *archetype);
void ecs_storage_release(archetype_storage *storage);

entity_id ecs_storage_create_entity(archetype_storage *storage);
void *ecs_storage_get_entity_component(archetype_storage *storage, entity_id ent_id, component_id comp_id);
void ecs_storage_destroy_entity(archetype_storage *storage, entity_id id);