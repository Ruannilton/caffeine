#include "ecs_query.h"
#include "../caffeine_memory.h"
#include "../ds/caffeine_vector.h"
#include "ecs_storage.h"

struct ecs_query
{
    const component_id *requiriments;
    uint32_t requiriments_count;
};

struct ecs_query_builder
{
    component_id *buffer;
    uint32_t count;
    uint32_t capacity;
};

cff_arr_impl(ecs_query_builder, component_id);

ecs_query_builder *ecs_query_builder_new()
{
    uint32_t capacity = 4;

    ecs_query_builder *builder = (ecs_query_builder *)CFF_ALLOC(sizeof(ecs_query_builder), "ECS_QUERY_BUILDER");

    if (builder == NULL)
    {
        return NULL;
    }

    ecs_query_builder_init(builder, capacity);

    return builder;
}

void ecs_query_builder_with_component(ecs_query_builder *const builder_mut_ref, component_id component)
{
    cff_arr_ordered_add(builder_mut_ref, component);
}

ecs_query *ecs_query_builder_build(const ecs_query_builder *const builder_ref)
{
    component_id *comps = NULL;
    CFF_ARR_COPY(builder_ref->buffer, comps, builder_ref->count);

    if (comps == NULL)
        return NULL;

    ecs_query *query = (ecs_query *)CFF_ALLOC(sizeof(ecs_query), "QUERY");

    if (query == NULL)
    {
        CFF_RELEASE(comps);
        return NULL;
    }

    query->requiriments = comps;
    query->requiriments_count = builder_ref->count;

    return query;
}

ecs_query *ecs_query_new_from_components(int count, const component_id *const components_ref)
{
    ecs_query *query = (ecs_query *)CFF_ALLOC(sizeof(ecs_query), "QUERY");
    query->requiriments = components_ref;
    query->requiriments_count = count;
    return query;
}

void ecs_query_release(const ecs_query *const query_owning)
{
    CFF_RELEASE(query_owning->requiriments);
    CFF_RELEASE(query_owning);
}

void *ecs_iterator_get_component_data(query_it it, component_id component)
{
    return ecs_storage_get_component_list(it, component);
}

entity_id *ecs_iterator_get_ids(query_it it)
{
    return ecs_storage_get_enetities_ids(it);
}

const component_id *ecs_query_get_components(const ecs_query *const query_ref)
{
    return query_ref->requiriments;
}

uint32_t ecs_query_get_count(const ecs_query *const query_ref)
{
    return query_ref->requiriments_count;
}