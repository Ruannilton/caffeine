#include "ecs_archetype_graph.h"
#include "../caffeine_memory.h"
#include "../caffeine_logging.h"

#define INVALID_ID ((uint32_t)-1)

typedef struct graph_node graph_node;

typedef struct
{
    uint32_t on_add;
    uint32_t on_remove;
} graph_edge;

struct graph_node
{
    uint64_t id;
    archetype_id arch_id;
    component_id *components;
    graph_edge *edges;
    uint32_t edge_count;
    uint32_t edge_capacity;
};

struct archetype_graph
{
    uint32_t count;
    uint32_t capacity;
    graph_node *nodes;
};

static void add_edge(const archetype_graph *const graph_ref, uint32_t from_node, component_id component, uint32_t to_node);
static uint32_t get_node_with(const archetype_graph *const graph_ref, uint32_t count, const component_id components[]);
static int get_edge_index(const archetype_graph *const graph_ref, uint32_t node_index, component_id component);
static void build_component_list(component_id result[], const component_id from[], uint64_t to_add, uint32_t from_len);
// static void debug_node(archetype_graph *graph, uint32_t node_index);
static uint32_t on_add_branch(const archetype_graph *const graph_ref, uint32_t node_index, uint32_t edge_index);
static bool add_component_to_node(const archetype_graph *const graph_ref, uint32_t node_index, component_id component, uint32_t on_add, uint32_t on_rem);
static uint32_t init_new_node(archetype_graph *graph, archetype_id arch_id, uint32_t from_node);
static uint32_t get_node_components(const archetype_graph *const graph_ref, uint32_t index, component_id const **out_mut_ref);
static void set_node_arch_id(const archetype_graph *const graph_ref, uint32_t index, archetype_id arch_id);

archetype_graph *ecs_archetype_graph_new()
{
    uint32_t capacity = 8;
    archetype_graph *graph = (archetype_graph *)CFF_ALLOC(sizeof(archetype_graph), "ARCHETYPE GRAPH");

    graph->nodes = (graph_node *)CFF_ALLOC(sizeof(graph_node) * capacity, "GRAPH NODES");
    graph->capacity = capacity;
    graph->count = 0;

    CFF_ZERO(graph->nodes, sizeof(graph_node) * capacity);

    init_new_node(graph, INVALID_ID, INVALID_ID);

    return graph;
}

void ecs_archetype_graph_release(const archetype_graph *const graph_owning)
{
    for (size_t i = 0; i < graph_owning->count; i++)
    {
        CFF_RELEASE(graph_owning->nodes[i].components);
        CFF_RELEASE(graph_owning->nodes[i].edges);
    }
    CFF_RELEASE(graph_owning->nodes);
    CFF_RELEASE(graph_owning);
}

void ecs_archetype_graph_add(archetype_graph *const graph_mut_ref, archetype_id id, uint32_t components_count, const component_id *const components_ref)
{
    uint32_t current_node = 0;
    component_id tmp_comps[components_count + 1];

    for (size_t i = 0; i < components_count; i++)
    {
        component_id component = components_ref[i];

        // caff_raw_log("[GRAPH] On Node:\n");
        // debug_node(graph, current_node);
        // caff_raw_log("[GRAPH] On Component: {%d}\n", component);

        int edge_pos = get_edge_index(graph_mut_ref, current_node, component);

        if (edge_pos == INVALID_ID)
        {
            // caff_raw_log("\t[GRAPH] Edge not Found\n");

            const component_id *current_components = NULL;
            uint32_t current_components_count = get_node_components(graph_mut_ref, current_node, &current_components);

            build_component_list(tmp_comps, current_components, component, current_components_count);

            uint32_t to_node = get_node_with(graph_mut_ref, current_components_count + 1, tmp_comps);

            if (to_node == INVALID_ID)
            {
                to_node = init_new_node(graph_mut_ref, INVALID_ID, current_node);
                // caff_raw_log("\t\t[GRAPH] Created Node\n\t");
                // debug_node(graph, to_node);
            }
            else
            {
                // caff_raw_log("\t\t[GRAPH] Found Node\n\t");
                // debug_node(graph, to_node);
            }

            add_edge(graph_mut_ref, current_node, component, to_node);
            // caff_raw_log("\t[GRAPH] Added Edge to Node\n\t");
            // debug_node(graph, to_node);
            current_node = to_node;
        }
        else
        {
            current_node = on_add_branch(graph_mut_ref, current_node, edge_pos);
        }
    }
    set_node_arch_id(graph_mut_ref, current_node, id);
}

uint32_t ecs_archetype_graph_find_with(const archetype_graph *const graph_ref, uint32_t count, const component_id components[], archetype_id **const out_mut_ref)
{

    archetype_id result[graph_ref->count];
    uint32_t result_count = 0;

    bool looked[graph_ref->count];
    for (size_t i = 0; i < graph_ref->count; i++)
    {
        looked[i] = false;
    }

    uint32_t to_look[graph_ref->count];
    to_look[0] = get_node_with(graph_ref, count, components);
    uint32_t to_look_count = 1;
    uint32_t to_look_index = 0;

    while (to_look_index != to_look_count)
    {
        uint32_t current_index = to_look[to_look_index];

        graph_node *current = graph_ref->nodes + current_index;

        if (looked[current->arch_id])
        {
            to_look_index++;
            continue;
        }

        for (size_t i = 0; i < current->edge_count; i++)
        {
            uint32_t to_add = current->edges[i].on_add;
            if (!looked[(graph_ref->nodes + to_add)->arch_id])
            {
                to_look[to_look_count] = to_add;
                to_look_count++;
            }
        }

        result[result_count] = current->arch_id;
        result_count++;

        looked[current->arch_id] = true;
        to_look_index++;
    }

    CFF_ARR_COPY(result, *out_mut_ref, result_count);
    return result_count;
}

static void set_node_arch_id(const archetype_graph *const graph_ref, uint32_t index, archetype_id arch_id)
{
    if (index > graph_ref->count)
        return;

    graph_node *node = graph_ref->nodes + index;
    node->arch_id = arch_id;
}

static uint32_t on_add_branch(const archetype_graph *const graph_ref, uint32_t node_index, uint32_t edge_index)
{
    if (edge_index == INVALID_ID)
        return INVALID_ID;
    graph_node *node = graph_ref->nodes + node_index;
    return node->edges[edge_index].on_add;
}

/*
 busca um nó que contenha todos os componentes do array de componentes informado, caso não encontre retorna nulo
*/
static uint32_t get_node_with(const archetype_graph *const graph_ref, uint32_t components_count, const component_id components[])
{
    uint32_t current_node = 0;
    for (size_t i = 0; i < components_count; i++)
    {
        component_id component = components[i];
        int edge_pos = get_edge_index(graph_ref, current_node, component);

        if (edge_pos == INVALID_ID)
            return INVALID_ID;

        current_node = on_add_branch(graph_ref, current_node, edge_pos);
    }

    return current_node;
}

static void add_edge(const archetype_graph *const graph_ref, uint32_t from_node, component_id component, uint32_t to_node)
{
    add_component_to_node(graph_ref, from_node, component, to_node, INVALID_ID);
    add_component_to_node(graph_ref, to_node, component, to_node, from_node);
}

/*busca o índice da aresta para o componente indicado, caso o nó não tenha uma aresta para o componente retorna -1*/
static int get_edge_index(const archetype_graph *const graph_ref, uint32_t node_index, component_id component)
{
    if (node_index > graph_ref->count)
        return INVALID_ID;

    graph_node *node = graph_ref->nodes + node_index;

    if (node->edge_count == 0 || node->edges == NULL)
        return INVALID_ID;

    for (size_t i = 0; i < node->edge_count; i++)
    {
        if (node->components[i] == component)
            return i;
    }
    return INVALID_ID;
}

/*adiciona um componente na lista de commponentes de maneira ordenada*/
static void build_component_list(component_id result[], const component_id from[], uint64_t to_add, uint32_t from_len)
{
    uint32_t result_index = 0;

    if (from_len == 0)
    {
        result[0] = to_add;
        return;
    }

    if (to_add < from[0])
    {
        result[result_index] = to_add;
        result_index++;
        for (size_t i = 0; i < from_len; i++)
        {
            result[result_index] = from[i];
            result_index++;
        }
        return;
    }
    if (to_add > from[from_len - 1])
    {
        for (size_t i = 0; i < from_len; i++)
        {
            result[result_index] = from[i];
            result_index++;
        }
        result[result_index] = to_add;
        result_index++;
        return;
    }

    size_t i = 0;
    for (; i < from_len; i++)
    {
        if (from[i] > to_add)
            break;
        result[result_index] = from[i];
        result_index++;
    }
    result[result_index] = to_add;
    result_index++;
    for (; i < from_len; i++)
    {
        result[result_index] = from[i];
        result_index++;
    }
}

// static void debug_node(archetype_graph *graph, uint32_t node_index)
// {
//     if (node_index > graph->count)
//         return;
//     graph_node *node = graph->nodes + node_index;

//     // caff_raw_log("{ \n\tid: %d\n\tarch_id: %d\n\tcomponents: [ ", node->id, node->arch_id);
//     for (size_t i = 0; i < node->edge_count; i++)
//     {
//         // caff_raw_log("%d, ", node->components[i]);
//     }
//     // caff_raw_log("]\n\tedges:[\n");
//     for (size_t i = 0; i < node->edge_count; i++)
//     {
//         uint32_t on_add_index = node->edges[i].on_add;

//         if (on_add_index == (uint32_t)INVALID_ID)
//         {
//             // caff_raw_log("\t\t{ %d {%d} (%d)-> _ {_} }\n", node->id, node->arch_id, node->components[i]);
//         }
//         else if (on_add_index < graph->count)
//         {
//             graph_node *on_add = graph->nodes + on_add_index;
//             // caff_raw_log("\t\t{ %d {%d} (%d)-> %d {%d} }\n", node->id, node->arch_id, node->components[i], on_add->id, on_add->arch_id);
//         }
//     }
//     // caff_raw_log("\t]\t\n}\n");
// }

static bool add_component_to_node(const archetype_graph *const graph_ref, uint32_t node_index, component_id component, uint32_t on_add, uint32_t on_rem)
{
    graph_node *node_ptr = graph_ref->nodes + node_index;

    if (node_ptr->edge_count == node_ptr->edge_capacity)
    { // TODO: handle allocation
        uint32_t new_capacity = node_ptr->edge_capacity * 2;
        node_ptr->edges = (graph_edge *)CFF_REALLOC(node_ptr->edges, new_capacity);
        node_ptr->components = (component_id *)CFF_REALLOC(node_ptr->components, new_capacity);
        node_ptr->edge_capacity = new_capacity;
    }

    uint32_t component_index = INVALID_ID;

    if (node_ptr->edge_count == 0)
    {
        component_index = 0;
        node_ptr->edge_count++;
    }
    else
    {
        uint32_t lenght = node_ptr->edge_count;
        // Find the correct position to insert 'component'
        for (uint32_t i = 0; i < lenght; i++)
        {
            if (component == node_ptr->components[i])
            {
                component_index = i;
                break;
            }

            if (component < node_ptr->components[i])
            {
                for (uint32_t j = lenght; j > i; j--)
                {
                    node_ptr->components[j] = node_ptr->components[j - 1];
                }

                // Insert 'component' at the correct position
                node_ptr->components[i] = component;
                component_index = i;
                node_ptr->edge_count++;
                break;
            }
        }
    }

    graph_edge *edge = node_ptr->edges + component_index;
    edge->on_add = on_add;
    edge->on_remove = on_rem;

    return true;
}

static uint32_t init_new_node(archetype_graph *const graph_mut_ref, archetype_id arch_id, uint32_t from_node)
{
    if (graph_mut_ref->count == graph_mut_ref->capacity)
    {
        uint32_t new_capacity = graph_mut_ref->capacity * 2;
        void *new_ptr = CFF_REALLOC(graph_mut_ref->nodes, new_capacity);
        if (new_ptr != NULL)
        {
            graph_mut_ref->nodes = (graph_node *)new_ptr;
            graph_mut_ref->capacity = new_capacity;
        }
        else
        {
            return INVALID_ID;
        }
    }

    uint32_t node_index = graph_mut_ref->count;
    graph_node *node_ptr = graph_mut_ref->nodes + node_index;

    node_ptr->id = node_index;
    node_ptr->arch_id = arch_id;
    node_ptr->edge_capacity = 4;
    node_ptr->edge_count = 0;

    node_ptr->components = (component_id *)CFF_ALLOC(sizeof(component_id) * node_ptr->edge_capacity, "NODE COMPONENTS");
    node_ptr->edges = (graph_edge *)CFF_ALLOC(sizeof(graph_edge) * node_ptr->edge_capacity, "NODE EDGES");
    graph_mut_ref->count++;

    component_id def_value = INVALID_ID;
    CFF_SET(&def_value, node_ptr->components, sizeof(component_id), sizeof(component_id) * node_ptr->edge_capacity);

    for (size_t i = 0; i < node_ptr->edge_capacity; i++)
    {
        node_ptr->edges[i].on_add = INVALID_ID;
        node_ptr->edges[i].on_remove = INVALID_ID;
    }

    if (from_node == INVALID_ID)
    {
        return node_index;
    }

    graph_node *from_node_ptr = graph_mut_ref->nodes + from_node;

    for (size_t i = 0; i < from_node_ptr->edge_count; i++)
    {
        component_id comp = from_node_ptr->components[i];
        add_component_to_node(graph_mut_ref, node_index, comp, node_index, from_node);
        add_component_to_node(graph_mut_ref, from_node, comp, node_index, from_node);
    }

    return node_index;
}

static uint32_t get_node_components(const archetype_graph *const graph_ref, uint32_t index, component_id const **out_mut_ref)
{
    if (index > graph_ref->count)
        return 0;

    if (out_mut_ref == NULL)
        return 0;

    graph_node *node = graph_ref->nodes + index;

    if (node == NULL)
    {
        *out_mut_ref = NULL;
        return 0;
    }

    *out_mut_ref = node->components;
    return node->edge_count;
}