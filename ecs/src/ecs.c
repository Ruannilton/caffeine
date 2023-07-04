#include <ecs.h>

cff_err_e ecs_init_world(ecs_world *world, cff_allocator_t allocator)
{
    world->component_id_counter = 0;
    world->allocator = allocator;
    world->components = cff_hashmap_create(sizeof(component_id), sizeof(ecs_component_info), 16, _compare_component_id, _compare_component_data, _hash_component_id, allocator);
    world->component_names = cff_hashmap_create(sizeof(cff_string), sizeof(component_id), 16, _compare_component_key, _compare_component_id, _hash_component_key, allocator);

    return CFF_ERR_NONE;
}
component_id ecs_register_component(ecs_world *world, cff_string name, cff_size size)
{
    ecs_component_info info = (ecs_component_info){
        .id = world->component_id_counter++,
        .name = cff_string_copy(name, world->allocator),
        .size = size,
    };

    if (cff_hashmap_add(world->components, (uintptr_t)(&info.id), (uintptr_t)&info, world->allocator))
    {
        if (cff_hashmap_add(world->component_names, (uintptr_t)&name, (uintptr_t)(&info.id), world->allocator))
        {
            return info.id;
        }
        cff_hashmap_remove(world->components, (uintptr_t)(&info.id), world->allocator);
    }

    return 0;
}

component_id ecs_get_component_id_from_name(ecs_world *world, cff_string name)
{
    component_id id = 0;

    if (cff_hashmap_get(world->component_names, (uintptr_t)(&name), (uintptr_t *)(&id)))
        return id;
    return 0;
}
ecs_component_info ecs_get_component_info(ecs_world *world, component_id id)
{
    ecs_component_info info = {0};

    if (cff_hashmap_get(world->component_names, (uintptr_t)(&id), (uintptr_t *)(&info)))
        return info;
    return (ecs_component_info){0};
}

static uint64_t _hash_component_key(uintptr_t data_ptr, cff_size data_size, uint64_t seed)
{
    char *str = ((cff_string *)data_ptr)->buffer;
    uint64_t hash = 5381 + seed;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

static uint64_t _hash_component_id(uintptr_t data_ptr, cff_size data_size, uint64_t seed)
{
    component_id *id_a = (component_id *)data_ptr;
    return *id_a + seed;
}

static bool _compare_component_key(uintptr_t a, uintptr_t b, cff_size datasize)
{
    cff_string *str_a = (cff_string *)a;
    cff_string *str_b = (cff_string *)b;

    return cff_string_compare(*str_a, *str_b);
}

static bool _compare_component_id(uintptr_t a, uintptr_t b, cff_size datasize)
{
    component_id *id_a = (component_id *)a;
    component_id *id_b = (component_id *)b;

    return id_a == id_b;
}

static bool _compare_component_data(uintptr_t a, uintptr_t b, cff_size datasize)
{
    ecs_component_info *comp_a = (ecs_component_info *)a;
    ecs_component_info *comp_b = (ecs_component_info *)b;

    return comp_a->id == comp_b->id;
}
