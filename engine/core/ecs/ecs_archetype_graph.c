#include "ecs_archetype_graph.h"
#include "../caffeine_memory.h"
#include "../caffeine_logging.h"

typedef struct graph_node graph_node;

typedef struct
{
    graph_node *on_add;
    graph_node *on_remove;
} graph_edge;

struct graph_node
{
    uint64_t id;
    archetype_id arch_id;
    component_id *components;
    graph_edge *edges;
    uint32_t edge_count;
};

struct archetype_graph
{
    uint32_t count;
    uint32_t capacity;
    graph_node *nodes;
};

static graph_node *get_node_with(archetype_graph *graph, uint32_t count, const component_id components[]);
static void insert_edge(graph_node *node, graph_edge edge, uint32_t at);
static graph_node *new_node(archetype_graph *graph, graph_node *from);
static uint32_t ordered_add(component_id *components, component_id component, uint32_t len);
static void add_edge(graph_node *from_node, component_id component, graph_node *to_node);
static int get_edge_index(graph_node *node, component_id component);
static void build_component_list(component_id result[], const component_id from[], uint64_t to_add, uint32_t from_len);
static void debug_node(graph_node *node);

archetype_graph *ecs_archetype_graph_new()
{
    uint32_t capacity = 8;
    archetype_graph *graph = (archetype_graph *)cff_mem_alloc(sizeof(archetype_graph));
    graph->nodes = (graph_node *)cff_mem_alloc(sizeof(graph_node) * capacity);
    graph->count = 1;
    graph->capacity = capacity;

    graph_node *first = graph->nodes;
    first->arch_id = INVALID_ID;
    first->edges = NULL;
    first->edge_count = 0;
    first->id = 0;

    return graph;
}

void ecs_archetype_graph_release(archetype_graph *graph)
{
    cff_mem_release(graph->nodes);
    cff_mem_release(graph);
}

void ecs_archetype_graph_add(archetype_graph *graph, archetype_id id, uint32_t count, const component_id components[])
{
    graph_node *node = graph->nodes;
    component_id tmp_comps[count + 1];

    for (size_t i = 0; i < count; i++)
    {
        component_id component = components[i];
        caff_raw_log("[GRAPH] On Node:\n");
        debug_node(node);

        caff_raw_log("[GRAPH] On Component: {%d}\n", component);

        int edge_pos = get_edge_index(node, component);

        if (edge_pos == -1)
        {
            caff_raw_log("\t[GRAPH] Edge not Found\n");
            build_component_list(tmp_comps, node->components, component, node->edge_count);

            graph_node *to_node = get_node_with(graph, node->edge_count + 1, tmp_comps);

            if (to_node == NULL)
            {
                to_node = new_node(graph, node);
                caff_raw_log("\t\t[GRAPH] Created Node\n\t");
                debug_node(to_node);
            }
            else
            {
                caff_raw_log("\t\t[GRAPH] Found Node\n\t");
                debug_node(to_node);
            }

            add_edge(node, component, to_node);
            caff_raw_log("\t[GRAPH] Added Edge to Node\n\t");
            debug_node(to_node);
            node = to_node;
        }
        else
        {
            node = node->edges[edge_pos].on_add;
        }
    }

    node->arch_id = id;
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

    graph_node *to_look[graph->count];
    to_look[0] = get_node_with(graph, count, components);
    uint32_t to_look_count = 1;
    uint32_t to_look_index = 0;

    while (to_look_index != to_look_count)
    {
        graph_node *current = to_look[to_look_index];

        if (looked[current->arch_id])
        {
            to_look_index++;
            continue;
        }

        for (size_t i = 0; i < current->edge_count; i++)
        {
            graph_node *to_add = current->edges[i].on_add;
            if (!looked[to_add->arch_id])
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

static graph_node *get_node_with(archetype_graph *graph, uint32_t count, const component_id components[])
{
    graph_node *node = graph->nodes;
    for (size_t i = 0; i < count; i++)
    {
        component_id component = components[i];
        int edge_pos = get_edge_index(node, component);

        if (edge_pos == -1)
            return NULL;

        node = node->edges[edge_pos].on_add;
    }

    return node;
}

static void insert_edge(graph_node *node, graph_edge edge, uint32_t at)
{
    if (at >= node->edge_count)
    {
        // Insert the edge at the end of the edges array
        node->edges[at] = edge;
    }
    else
    {
        // Shift elements to the right to make room for the new edge
        for (uint32_t i = node->edge_count; i > at; i--)
        {
            node->edges[i] = node->edges[i - 1];
        }

        // Insert the edge at the specified position
        node->edges[at] = edge;
    }
}

static graph_node *new_node(archetype_graph *graph, graph_node *from)
{
    if (graph->count == graph->capacity)
    {
        cff_resize_arr(graph->nodes, graph->capacity * 2);
        graph->capacity *= 2;
    }

    graph_node *new = graph->nodes + graph->count;

    new->arch_id = INVALID_ID;
    new->edge_count = from->edge_count;
    new->components = NULL;
    new->edges = NULL;
    new->id = graph->count;
    graph->count++;

    if (from->edge_count > 0)
    {
        cff_arr_copy(from->components, new->components, from->edge_count);
        cff_arr_copy(from->edges, new->edges, from->edge_count);
    }

    for (size_t i = 0; i < new->edge_count; i++)
    {
        new->edges[i].on_add = new;
        new->edges[i].on_remove = NULL;
    }

    return new;
}

static uint32_t ordered_add(component_id *components, component_id component, uint32_t len)
{
    uint32_t i;

    // Find the correct position to insert 'component'
    for (i = 0; i < len; i++)
    {
        if (component < components[i])
        {
            break;
        }
    }

    // Shift elements to the right to make room for 'component'
    for (uint32_t j = len; j > i; j--)
    {
        components[j] = components[j - 1];
    }

    // Insert 'component' at the correct position
    components[i] = component;

    return i;
}

static void resize_edges(graph_node *node)
{
    if (node->edges == NULL)
        node->edges = cff_new_arr(graph_edge, 1);
    else
        cff_resize_arr(node->edges, node->edge_count + 1);

    if (node->components == NULL)
        node->components = cff_new_arr(component_id, 1);
    else
        cff_resize_arr(node->components, node->edge_count + 1);

    node->components[node->edge_count] = INVALID_ID;

    node->edge_count++;
}

static void add_edge(graph_node *from_node, component_id component, graph_node *to_node)
{
    resize_edges(from_node);
    resize_edges(to_node);

    uint32_t from_edge_pos = ordered_add(from_node->components, component, from_node->edge_count);
    uint32_t to_edge_pos = ordered_add(to_node->components, component, to_node->edge_count);

    graph_edge new_edge = {.on_add = to_node, .on_remove = from_node};

    insert_edge(from_node, new_edge, from_edge_pos);
    insert_edge(to_node, new_edge, to_edge_pos);
}

static int get_edge_index(graph_node *node, component_id component)
{
    for (size_t i = 0; i < node->edge_count; i++)
    {
        if (node->components[i] == component)
            return i;
    }
    return -1;
}

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

static void debug_node(graph_node *node)
{
    caff_raw_log("{ \n\tid: %d\n\tarch_id: %d\n\tcomponents: [ ", node->id, node->arch_id);
    for (size_t i = 0; i < node->edge_count; i++)
    {
        caff_raw_log("%d, ", node->components[i]);
    }
    caff_raw_log("]\n\tedges:[\n");
    for (size_t i = 0; i < node->edge_count; i++)
    {
        caff_raw_log("\t\t{ %d {%d} (%d)-> %d {%d} }\n", node->id, node->arch_id, node->components[i], node->edges[i].on_add->id, node->edges[i].on_add->arch_id);
    }
    caff_raw_log("\t]\t\n}\n");
}