#pragma once

#include <caffeine_core.h>
#include <caffeine_hashmap.h>
#include <caffeine_string.h>
#include <caffeine_vector.h>


typedef uint64_t component_id;
typedef uint64_t archetype_id;
typedef cff_vector_s component_set;

typedef struct {
  component_id id;
  cff_string name;
  cff_size size;
} ecs_component_info;

typedef struct {
  cff_vector_s components;
} ecs_archetype;

typedef struct {
  cff_allocator_t allocator;
  cff_hashmap_s components;
  cff_hashmap_s component_names;
  uint64_t component_id_counter;
} ecs_world;

// world
cff_err_e ecs_init_world(ecs_world *world, cff_allocator_t allocator);

// component
component_id ecs_register_component(ecs_world *world, cff_string name,
                                    cff_size size);
component_id ecs_get_component_id_from_name(ecs_world *world, cff_string name);
ecs_component_info ecs_get_component_info(ecs_world *world, component_id id);
