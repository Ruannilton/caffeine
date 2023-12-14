#pragma once

#include "../../caffeine_types.h"
#include "ecs_types.h"

typedef struct ecs_query_builder ecs_query_builder;

CAFF_API ecs_query_builder *ecs_query_builder_new();
CAFF_API void ecs_query_builder_with_component(ecs_query_builder *const builder_mut_ref, component_id component);
CAFF_API ecs_query *ecs_query_builder_build(const ecs_query_builder *const builder_ref);
CAFF_API void ecs_query_builder_release(ecs_query_builder *builder_owning);

const component_id *ecs_query_get_components(const ecs_query *const query_ref);
uint32_t ecs_query_get_count(const ecs_query *const query_ref);
void ecs_query_release(const ecs_query *const query_owning);

CAFF_API void *ecs_iterator_get_component_data(query_it it, component_id component);
CAFF_API entity_id *ecs_iterator_get_ids(query_it it);