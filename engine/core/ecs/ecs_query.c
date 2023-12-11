#include "ecs_query.h"
#include "../caffeine_memory.h"

struct ecs_query
{
    const component_id *requiriments;
    uint32_t requiriments_count;
};

ecs_query *ecs_query_new(int count, const component_id *const components_owning)
{
    ecs_query *query = (ecs_query *)CFF_ALLOC(sizeof(ecs_query), "QUERY");
    query->requiriments = components_owning;
    query->requiriments_count = count;
    return query;
}

void ecs_query_release(const ecs_query *const query_owning)
{
    CFF_RELEASE(query_owning->requiriments);
    CFF_RELEASE(query_owning);
}

const component_id *ecs_query_get_components(const ecs_query *const query_ref)
{
    return query_ref->requiriments;
}

uint32_t ecs_query_get_count(const ecs_query *const query_ref)
{
    return query_ref->requiriments_count;
}