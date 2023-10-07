#include "archetype_map.h"
#include "archetype_graph.h"

#define LOG_SCOPE "ECS::Archetype Map - "

typedef struct
{
    uint32_t capacity, count;
    archetype *buffer;
    uint32_t *hash;
    archetype_id *id;
    uint8_t *used;
} archetype_map;

bool archetype_compare(archetype *a, archetype *b);
static bool _map_resize();
static bool _internal_map_add(archetype_map *amap, archetype value, archetype_id *out);

static archetype_map map;

static uint32_t _hash_archetype(archetype arche, int gen, uint32_t max)
{
    uint32_t hash = gen;
    uint32_t prime = 31; // A prime number to help reduce collisions

    for (size_t i = 0; i < arche.count; i++)
    {
        // Improved hashing algorithm using a prime number
        hash = hash * prime + arche.buffer[i];
    }

    return hash % max;
}

static bool _internal_map_init(archetype_map *amap, uint32_t capacity)
{
    amap->capacity = capacity;
    amap->count = 0;
    amap->buffer = cff_new_arr(archetype, amap->capacity);

    if (amap->buffer == NULL)
    {
        return false;
    }

    amap->hash = cff_new_arr(uint32_t, amap->capacity);
    if (amap->hash == NULL)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to alloc\n");
        cff_mem_release(amap->buffer);
        return false;
    }

    amap->used = cff_new_arr(uint8_t, amap->capacity);
    if (amap->hash == NULL)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to alloc\n");
        cff_mem_release(amap->buffer);
        cff_mem_release(amap->hash);
        return false;
    }

    amap->id = cff_new_arr(archetype_id, amap->capacity);
    if (amap->id == NULL)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to alloc\n");
        cff_mem_release(amap->used);
        cff_mem_release(amap->buffer);
        cff_mem_release(amap->hash);

        return false;
    }

    cff_mem_zero(amap->buffer, sizeof(archetype), amap->capacity * sizeof(archetype));
    cff_mem_zero(amap->id, sizeof(archetype_id), amap->capacity * sizeof(archetype_id));
    cff_mem_zero(amap->used, sizeof(uint8_t), amap->capacity * sizeof(uint8_t));
    cff_mem_zero(amap->hash, sizeof(uint32_t), amap->capacity * sizeof(uint32_t));

    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Initialized\n");
    return true;
}

static bool _internal_map_add(archetype_map *amap, archetype value, archetype_id *out)
{
    if ((float)amap->count / amap->capacity > 0.75f)
    {
        if (!_map_resize())
        {
            *out = INVALID_ARCHETYPE_ID;
            caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to resize\n");
            caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Archetype not registered\n");
            return false;
        }
    }

    int generation = 0;
    uint32_t hash = _hash_archetype(value, generation, amap->capacity);

    while (amap->used[hash])
    {
        if (archetype_compare(&value, &amap->buffer[hash]))
        {
            *out = amap->id[hash];
            return true;
        }
        caff_log(LOG_LEVEL_DEBUG, LOG_SCOPE "Colision on add\n");
        generation++;
        hash = _hash_archetype(value, generation, amap->capacity);
    }

    uint32_t id = amap->count;

    amap->buffer[hash] = value;
    amap->used[hash] = 1;
    amap->hash[id] = hash;
    amap->id[hash] = id;
    amap->count++;

    *out = id;

    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Archetype registered\n");
    return true;
}

static bool _map_resize()
{
    archetype_map new_map;
    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Resizing\n");
    if (!_internal_map_init(&new_map, map.capacity * 2))
        return false;

    for (uint32_t i = 0; i < map.count; i++)
    {
        uint32_t hash = map.hash[i];

        if (!map.used[hash])
            continue;

        archetype arch = map.buffer[hash];
        archetype_id out;
        _internal_map_add(&new_map, arch, &out);
    }

    map_release(map);
    map = new_map;
    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Resized\n");
    return true;
}

bool map_init(uint32_t capacity)
{
    return _internal_map_init(&map, capacity);
}

void map_release()
{
    if (map.buffer)
    {
        for (uint32_t i = 0; i < map.count; i++)
        {
            uint32_t hash = map.hash[i];

            if (!map.used[hash])
                continue;

            archetype *arch = map.buffer + hash;
            if (arch)
                archetype_release(arch);
        }

        cff_mem_release(map.buffer);
    }
    if (map.hash)
        cff_mem_release(map.hash);
    if (map.used)
        cff_mem_release(map.used);
    if (map.id)
        cff_mem_release(map.id);

    map = (archetype_map){0};
    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Released\n");
    graph_release();
}

bool map_add(archetype value, archetype_id *out)
{
    return _internal_map_add(&map, value, out);
}

archetype *map_get(archetype_id id)
{
    if (id >= map.capacity)
        return NULL;
    uint32_t hash = map.hash[id];

    if (!map.used[hash])
        return NULL;

    return map.buffer + hash;
}

bool map_build()
{
    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Building\n");
    uint32_t arch_count = map.count;

    bool graph_created = graph_init(arch_count);

    if (!graph_created)
    {
        caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Failed to init graph\n");
        return false;
    }

    for (size_t i = 0; i < map.capacity; i++)
    {
        if (map.used[i])
        {
            archetype *arch = map.buffer + i;
            archetype_id id = map.id[i];

            if (!graph_add_archetype(id, arch))
            {
                graph_release();
                caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Failed to register archetype to graph\n");
                return false;
            }
        }
    }
    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Builded\n");
    return true;
}