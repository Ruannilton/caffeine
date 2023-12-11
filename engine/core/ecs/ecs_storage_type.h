#pragma once

#include "ecs_types.h"

struct ecs_storage
{
    const size_t *component_sizes;
    const component_id *components;
    uint32_t component_count;

    uint32_t entity_count;
    uint32_t entity_capacity;
    entity_id *entities;
    void **entity_data;
};