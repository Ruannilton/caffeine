#pragma once

#include "ecs_types.h"

typedef struct ecs_storage ecs_storage;

typedef struct
{
    int row;
    ecs_storage *storage;
} entity_record;

typedef struct entity_index entity_index;

entity_index *ecs_entity_index_new(uint32_t capacity);
void ecs_entity_index_release(entity_index *index);

entity_id ecs_entity_index_new_entity(entity_index *index);

void ecs_entity_index_set_entity(entity_index *index, entity_id id, int row, ecs_storage *storage);
entity_record ecs_entity_index_get_entity(entity_index *index, entity_id id);
void ecs_entity_index_remove_entity(entity_index *index, entity_id id);