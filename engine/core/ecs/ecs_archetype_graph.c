#include "ecs_archetype_graph.h"
#include "../caffeine_memory.h"
#include "../caffeine_logging.h"

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

static void add_edge(archetype_graph *graph, uint32_t from_node, component_id component, uint32_t to_node);
static uint32_t get_node_with(archetype_graph *graph, uint32_t count, const component_id components[]);
static int get_edge_index(archetype_graph *graph, uint32_t node_index, component_id component);
static void build_component_list(component_id result[], const component_id from[], uint64_t to_add, uint32_t from_len);
static void debug_node(archetype_graph *graph, uint32_t node_index);
static uint32_t on_add_branch(archetype_graph *graph, uint32_t node_index, uint32_t edge_index);
static bool add_component_to_node(archetype_graph *graph, uint32_t node_index, component_id component, uint32_t on_add, uint32_t on_rem);
static uint32_t init_new_node(archetype_graph *graph, archetype_id arch_id, uint32_t from_node);
static uint32_t get_node_components(archetype_graph *graph, uint32_t index, component_id **out_ref);
static void set_node_arch_id(archetype_graph *graph, uint32_t index, archetype_id arch_id);

archetype_graph *ecs_archetype_graph_new()
{
    uint32_t capacity = 8;
    archetype_graph *graph = (archetype_graph *)cff_mem_alloc(sizeof(archetype_graph));

    graph->nodes = (graph_node *)cff_mem_alloc(sizeof(graph_node) * capacity);
    graph->capacity = capacity;
    graph->count = 0;

    cff_mem_zero(graph->nodes, sizeof(graph_node) * capacity);

    init_new_node(graph, INVALID_ID, (uint32_t)-1);

    return graph;
}

void ecs_archetype_graph_release(archetype_graph *graph)
{
    cff_mem_release(graph->nodes);
    cff_mem_release(graph);
}

void ecs_archetype_graph_add(archetype_graph *graph, archetype_id id, uint32_t components_count, const component_id components[])
{
    uint32_t current_node = 0;
    component_id tmp_comps[components_count + 1];

    for (size_t i = 0; i < components_count; i++)
    {
        component_id component = components[i];

        caff_raw_log("[GRAPH] On Node:\n");
        debug_node(graph, current_node);
        caff_raw_log("[GRAPH] On Component: {%d}\n", component);

        int edge_pos = get_edge_index(graph, current_node, component);

        if (edge_pos == -1)
        {
            caff_raw_log("\t[GRAPH] Edge not Found\n");

            component_id *current_components = NULL;
            uint32_t current_components_count = get_node_components(graph, current_node, &current_components);

            build_component_list(tmp_comps, current_components, component, current_components_count);

            uint32_t to_node = get_node_with(graph, current_components_count + 1, tmp_comps);

            if (to_node == -1)
            {
                to_node = init_new_node(graph, INVALID_ID, current_node);
                caff_raw_log("\t\t[GRAPH] Created Node\n\t");
                debug_node(graph, to_node);
            }
            else
            {
                caff_raw_log("\t\t[GRAPH] Found Node\n\t");
                debug_node(graph, to_node);
            }

            add_edge(graph, current_node, component, to_node);
            caff_raw_log("\t[GRAPH] Added Edge to Node\n\t");
            debug_node(graph, to_node);
            current_node = to_node;
        }
        else
        {
            current_node = on_add_branch(graph, current_node, edge_pos);
        }
    }
    set_node_arch_id(graph, current_node, id);
}

uint32_t ecs_archetype_graph_find_with(archetype_graph *graph, uint32_t count, const component_id components[], archetype_id *out)
{

    archetype_id result[graph->count];
    uint32_t result_count = 0;

    bool looked[graph->count];
    for (size_t i = 0; i < graph->count; i++)
    {
        looked[i] = false;
    }

    uint32_t to_look[graph->count];
    to_look[0] = get_node_with(graph, count, components);
    uint32_t to_look_count = 1;
    uint32_t to_look_index = 0;

    while (to_look_index != to_look_count)
    {
        uint32_t current_index = to_look[to_look_index];

        graph_node *current = graph->nodes + current_index;

        if (looked[current->arch_id])
        {
            to_look_index++;
            continue;
        }

        for (size_t i = 0; i < current->edge_count; i++)
        {
            uint32_t to_add = current->edges[i].on_add;
            if (!looked[(graph->nodes + to_add)->arch_id])
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

    cff_arr_copy(result, out, result_count);
    return result_count;
}

static void set_node_arch_id(archetype_graph *graph, uint32_t index, archetype_id arch_id)
{
    if (index > graph->count)
        return;

    graph_node *node = graph->nodes + index;
    node->arch_id = arch_id;
}

static uint32_t on_add_branch(archetype_graph *graph, uint32_t node_index, uint32_t edge_index)
{
    if (edge_index == -1)
        return -1;
    graph_node *node = graph->nodes + node_index;
    return node->edges[edge_index].on_add;
}

/*
 busca um nó que contenha todos os componentes do array de componentes informado, caso não encontre retorna nulo
*/
static uint32_t get_node_with(archetype_graph *graph, uint32_t components_count, const component_id components[])
{
    uint32_t current_node = 0;
    for (size_t i = 0; i < components_count; i++)
    {
        component_id component = components[i];
        int edge_pos = get_edge_index(graph, current_node, component);

        if (edge_pos == -1)
            return (uint32_t)-1;

        current_node = on_add_branch(graph, current_node, edge_pos);
    }

    return current_node;
}

static void add_edge(archetype_graph *graph, uint32_t from_node, component_id component, uint32_t to_node)
{
    add_component_to_node(graph, from_node, component, to_node, (uint32_t)-1);
    add_component_to_node(graph, to_node, component, to_node, from_node);
}

/*busca o índice da aresta para o componente indicado, caso o nó não tenha uma aresta para o componente retorna -1*/
static int get_edge_index(archetype_graph *graph, uint32_t node_index, component_id component)
{
    if (node_index > graph->count)
        return -1;

    graph_node *node = graph->nodes + node_index;

    if (node->edge_count == 0 || node->edges == NULL)
        return -1;

    for (size_t i = 0; i < node->edge_count; i++)
    {
        if (node->components[i] == component)
            return i;
    }
    return -1;
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

static void debug_node(archetype_graph *graph, uint32_t node_index)
{
    if (node_index > graph->count)
        return;
    graph_node *node = graph->nodes + node_index;

    caff_raw_log("{ \n\tid: %d\n\tarch_id: %d\n\tcomponents: [ ", node->id, node->arch_id);
    for (size_t i = 0; i < node->edge_count; i++)
    {
        caff_raw_log("%d, ", node->components[i]);
    }
    caff_raw_log("]\n\tedges:[\n");
    for (size_t i = 0; i < node->edge_count; i++)
    {
        uint32_t on_add_index = node->edges[i].on_add;

        if (on_add_index == (uint32_t)-1)
        {
            caff_raw_log("\t\t{ %d {%d} (%d)-> _ {_} }\n", node->id, node->arch_id, node->components[i]);
        }
        else if (on_add_index < graph->count)
        {
            graph_node *on_add = graph->nodes + on_add_index;
            caff_raw_log("\t\t{ %d {%d} (%d)-> %d {%d} }\n", node->id, node->arch_id, node->components[i], on_add->id, on_add->arch_id);
        }
    }
    caff_raw_log("\t]\t\n}\n");
}

static bool add_component_to_node(archetype_graph *graph, uint32_t node_index, component_id component, uint32_t on_add, uint32_t on_rem)
{
    graph_node *node_ptr = graph->nodes + node_index;

    if (node_ptr->edge_count == node_ptr->edge_capacity)
    { // TODO: handle allocation
        uint32_t new_capacity = node_ptr->edge_capacity * 2;
        node_ptr->edges = (graph_edge *)cff_mem_realloc(node_ptr->edges, new_capacity);
        node_ptr->components = (component_id *)cff_mem_realloc(node_ptr->components, new_capacity);
        node_ptr->edge_capacity = new_capacity;
    }

    uint32_t component_index = -1;

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

static uint32_t init_new_node(archetype_graph *graph, archetype_id arch_id, uint32_t from_node)
{
    if (graph->count == graph->capacity)
    {
        uint32_t new_capacity = graph->capacity * 2;
        void *new_ptr = cff_mem_realloc(graph->nodes, new_capacity);
        if (new_ptr != NULL)
        {
            graph->nodes = (graph_node *)new_ptr;
            graph->capacity = new_capacity;
        }
        else
        {
            return (uint32_t)(-1);
        }
    }

    uint32_t node_index = graph->count;
    graph_node *node_ptr = graph->nodes + node_index;

    node_ptr->id = node_index;
    node_ptr->arch_id = arch_id;
    node_ptr->edge_capacity = 4;
    node_ptr->edge_count = 0;

    node_ptr->components = (component_id *)cff_mem_alloc(sizeof(component_id) * node_ptr->edge_capacity);
    node_ptr->edges = (graph_edge *)cff_mem_alloc(sizeof(graph_edge) * node_ptr->edge_capacity);
    graph->count++;

    component_id def_value = INVALID_ID;
    cff_mem_set(&def_value, node_ptr->components, sizeof(component_id), sizeof(component_id) * node_ptr->edge_capacity);

    for (size_t i = 0; i < node_ptr->edge_capacity; i++)
    {
        node_ptr->edges[i].on_add = (uint32_t)-1;
        node_ptr->edges[i].on_remove = (uint32_t)-1;
    }

    if (from_node == (uint32_t)-1)
    {
        return node_index;
    }

    graph_node *from_node_ptr = graph->nodes + from_node;

    for (size_t i = 0; i < from_node_ptr->edge_count; i++)
    {
        component_id comp = from_node_ptr->components[i];
        add_component_to_node(graph, node_index, comp, node_index, from_node);
        add_component_to_node(graph, from_node, comp, node_index, from_node);
    }

    return node_index;
}

static uint32_t get_node_components(archetype_graph *graph, uint32_t index, component_id **out_ref)
{
    if (index > graph->count)
        return 0;

    if (out_ref == NULL)
        return 0;

    graph_node *node = graph->nodes + index;

    if (node == NULL)
    {
        *out_ref = NULL;
        return 0;
    }

    *out_ref = node->components;
    return node->edge_count;
}