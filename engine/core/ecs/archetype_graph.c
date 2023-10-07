#include "archetype_graph.h"
#include "archetype_storage.h"
#include "../caffeine_memory.h"

#define LOG_SCOPE "ECS::Archetype Graph - "

struct archetype_node_t;

typedef struct
{
    component_id component;
    struct archetype_node_t *add;
    struct archetype_node_t *rem;
} archetype_edge;

typedef struct archetype_node_t
{
    archetype_storage storage;
    archetype_edge *edges;
    uint32_t edge_count;
    uint32_t edge_capacity;
} archetype_node;

static archetype_node *nodes;
static uint32_t nodes_count;

static archetype_id *entity_to_node_buffer;
static uint32_t entity_to_node_count;
static uint32_t entity_to_node_capacity;

static bool init_node(archetype_node *node, archetype_id id, archetype *archetype)
{
    bool storage_created = ecs_storage_init(&node->storage, id, archetype);
    uint32_t component_count = archetype->count;

    if (!storage_created)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to initialize storage for archetype: %u\n", id);
        return false;
    }

    archetype_edge *edges = cff_new_arr(archetype_edge, component_count);

    if (edges == NULL)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to alloc node for archetype: %u\n", id);
        ecs_storage_release(&node->storage);
        return false;
    }

    cff_mem_zero(edges, sizeof(archetype_edge), sizeof(archetype_edge) * component_count);

    node->edges = edges;
    node->edge_count = 0;
    node->edge_capacity = component_count;

    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Node allocated for archetype: %u\n", id);

    return true;
}

static void release_node(archetype_node *node)
{
    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Node released for archetype: %u\n", node->storage.storage_header.arch_id);

    ecs_storage_release(&(node->storage));
    cff_mem_release(node->edges);
}

inline static archetype_node *get_storage_node_from_arch(archetype_id id)
{
    if (id >= nodes_count)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Node not found for archetype: %u\n", id);
        return NULL;
    }
    return &(nodes[id]);
}

inline static archetype_id get_archetype_from_entity(entity_id id)
{
    if (id >= entity_to_node_count)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Node not found archetype for entity: %u\n", id);
        return INVALID_ARCHETYPE_ID;
    }
    return entity_to_node_buffer[id];
}

inline static void register_entity_to_arch(entity_id e_id, archetype_id a_id)
{
    if (entity_to_node_count == entity_to_node_capacity)
    {
        entity_to_node_buffer = cff_resize_arr(entity_to_node_buffer, entity_to_node_capacity * 2);

        if (entity_to_node_buffer == NULL)
            caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to resize\n");

        entity_to_node_capacity *= 2;
    }

    entity_to_node_buffer[e_id] = a_id;
    entity_to_node_count++;
}

bool graph_init(uint32_t archetype_count)
{
    nodes = cff_new_arr(archetype_node, archetype_count);
    nodes_count = archetype_count;

    if (nodes == NULL)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to allocated\n");
        return false;
    }

    entity_to_node_count = 0;
    entity_to_node_capacity = 4;
    entity_to_node_buffer = cff_new_arr(archetype_id, entity_to_node_capacity);

    if (entity_to_node_buffer == NULL)
    {
        cff_mem_release(nodes);
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to allocated\n");
        return false;
    }

    return true;
}

void graph_release()
{
    for (size_t i = 0; i < nodes_count; i++)
    {
        release_node(nodes + i);
    }

    cff_mem_release(nodes);
}

// TODO: create paths betwen existent nodes and new
bool graph_add_archetype(archetype_id id, archetype *archetype)
{
    archetype_node *node = get_storage_node_from_arch(id);

    if (node == NULL)
    {
        return false;
    }

    if (!init_node(node, id, archetype))
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to initialize node for archetype: %u\n", id);
        return false;
    }

    return true;
}

entity_id graph_alloc_entity(archetype_id arch_id)
{
    archetype_node *node = get_storage_node_from_arch(arch_id);

    if (node == NULL)
        return INVALID_ENTITY_ID;

    archetype_storage *storage = &(node->storage);

    if (storage == NULL)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Node storage is null for archetype: %u \n", arch_id);
        return INVALID_ENTITY_ID;
    }

    entity_id id = ecs_storage_create_entity(storage);

    register_entity_to_arch(id, arch_id);

    return id;
}

void *graph_get_entity_component(entity_id ent_id, component_id comp_id)
{
    archetype_id arch_id = get_archetype_from_entity(ent_id);

    if (arch_id == INVALID_ARCHETYPE_ID)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "No archetype found to entity: %u \n", ent_id);
        return NULL;
    }

    archetype_node *node = get_storage_node_from_arch(arch_id);

    if (node == NULL)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "No archetype node found to archetype: %u \n", arch_id);
        return NULL;
    }

    archetype_storage *storage = &(node->storage);

    if (storage == NULL)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Node storage is null for archetype: %u \n", arch_id);
        return NULL;
    }

    return ecs_storage_get_entity_component(storage, ent_id, comp_id);
}