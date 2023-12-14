#include "ecs_system_index.h"
#include "ecs_query.h"
#include "../ds/caffeine_vector.h"
#include "../ds/caffeine_hashmap.h"
#include "ecs_storage_index.h"
#include "ecs_storage.h"

typedef uint32_t query_id;
typedef struct query_runner query_runner;

cff_arr_dcltype(archetype_list, archetype_id);
cff_arr_impl(archetype_list, archetype_id);

struct query_runner
{
    archetype_list archetypes;
    ecs_system system;
};

cff_arr_dcltype(query_list, ecs_query *);
cff_arr_impl(query_list, ecs_query *);

cff_arr_dcltype(runner_list, query_runner);
cff_arr_impl(runner_list, query_runner);

cff_hash_dcltype(query_map, ecs_query *, query_id);
cff_hash_impl(query_map, ecs_query *, query_id);

static uint32_t hash_key_fn(ecs_query **key, uint32_t seed)
{
    const component_id *components_ref = ecs_query_get_components(*key);
    uint32_t count = ecs_query_get_count(*key);

    uint32_t hash_value = seed;

    for (size_t i = 0; i < count; i++)
    {
        // Mix the bits of the current integer into the hash value
        hash_value ^= components_ref[i];
        hash_value *= 0x9e3779b1; // A prime number for mixing
    }

    return hash_value;
}

static bool cmp_key_fn(ecs_query **key_a, ecs_query **key_b)
{
    uint32_t count_a = ecs_query_get_count(*key_a);
    uint32_t count_b = ecs_query_get_count(*key_b);

    const component_id *components_ref_a = ecs_query_get_components(*key_a);
    const component_id *components_ref_b = ecs_query_get_components(*key_b);

    if (count_a != count_b)
        return false;

    for (size_t i = 0; i < count_a; i++)
    {
        if (components_ref_a[i] != components_ref_b[i])
            return false;
    }
    return true;
}

static bool cmp_data_fn(query_id *value_a, query_id *value_b)
{
    return *value_a == *value_b;
}

struct system_index
{
    query_map query_index;
    query_list queries;
    runner_list runners;
    const storage_index *storage_index;
};

static void query_runner_init(query_runner *runner, ecs_system system, archetype_id *archetypes, uint32_t lenght);
static void query_runner_release(query_runner *runner);
// static void query_runner_add_arch(query_runner *runner, archetype_id archetype);
// static void query_runner_rem_arch(query_runner *runner, archetype_id archetype);

system_index *ecs_system_index_new(const storage_index *storage_index, const uint32_t capacity)
{
    if (storage_index == NULL)
        return NULL;

    system_index *index = (system_index *)CFF_ALLOC(sizeof(system_index), "SYSTEM INDEX");
    if (index == NULL)
        return NULL;

    query_map_init(&(index->query_index), capacity, hash_key_fn, cmp_key_fn, cmp_data_fn);

    query_list_init(&(index->queries), capacity);

    runner_list_init(&(index->runners), capacity);

    index->storage_index = storage_index;

    return index;
}

void ecs_system_index_release(system_index *index)
{
    for (size_t i = 0; i < index->queries.count; i++)
    {
        ecs_query **query = query_list_get_ref(&(index->queries), i);
        if (query)
        {
            ecs_query_release(*query);
        }
    }

    query_map_release(&(index->query_index));

    query_list_release(&(index->queries));

    for (size_t i = 0; i < index->runners.count; i++)
    {
        query_runner *runner = runner_list_get_ref(&(index->runners), i);
        if (runner != NULL)
            query_runner_release(runner);
    }

    runner_list_release(&(index->runners));

    CFF_RELEASE(index);
}

void ecs_system_index_add(system_index *index, ecs_query *query, archetype_id *archetypes, uint32_t archetypes_count, ecs_system system)
{

    if (query_map_exist(&(index->query_index), query) != 0)
        return;

    query_id id = 0;
    query_list_add_i(&(index->queries), query, &id);

    query_map_add(&(index->query_index), query, id);

    query_runner runner = {0};

    query_runner_init(&runner, system, archetypes, archetypes_count);

    runner_list_add_at(&(index->runners), runner, id);
}

void ecs_system_index_add_archetype(system_index *index, archetype_id archetype)
{
    (void)index;
    (void)archetype;
}

void ecs_system_step(system_index *index)
{
    for (size_t i = 0; i < index->runners.count; i++)
    {
        query_runner *runner = runner_list_get_ref(&(index->runners), i);
        if (runner == NULL)
            continue;

        for (size_t j = 0; j < runner->archetypes.count; j++)
        {
            archetype_id arch = archetype_list_get(&(runner->archetypes), j);
            query_it it = ecs_storage_index_get(index->storage_index, arch);
            runner->system(it, ecs_storage_count(it));
        }
    }
}

static void query_runner_init(query_runner *runner, ecs_system system, archetype_id *archetypes, uint32_t lenght)
{
    if (runner == NULL)
        return;

    archetype_list_init(&(runner->archetypes), lenght);

    for (size_t i = 0; i < lenght; i++)
    {
        archetype_id arch = archetypes[i];
        archetype_list_add(&(runner->archetypes), arch);
    }

    runner->system = system;
}

static void query_runner_release(query_runner *runner)
{
    archetype_list_release(&(runner->archetypes));
    runner->system = NULL;
}

// static void query_runner_add_arch(query_runner *runner, archetype_id archetype)
// {
//     archetype_list_add(&(runner->archetypes), archetype);
// }
// static void query_runner_rem_arch(query_runner *runner, archetype_id archetype)
// {
//     archetype_list_remove(&(runner->archetypes), archetype);
// }
