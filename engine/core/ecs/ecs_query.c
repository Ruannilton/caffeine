#include "ecs_query.h"
#include "../caffeine_memory.h"

struct ecs_query
{
    component_id *requiriments;
    uint32_t requiriments_count;
};

ecs_query *ecs_query_new(int count, component_id *components)
{
    ecs_query *query = (ecs_query *)CFF_ALLOC(sizeof(ecs_query), "QUERY");
    query->requiriments = components;
    query->requiriments_count = count;
    return query;
}

void ecs_query_release(ecs_query *query)
{
    CFF_RELEASE(query->requiriments);
    CFF_RELEASE(query);
}

component_id *ecs_query_get_components(ecs_query *query)
{
    return query->requiriments;
}

uint32_t ecs_query_get_count(ecs_query *query)
{
    return query->requiriments_count;
}