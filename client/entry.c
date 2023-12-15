
#include <caffeine_entry.h>

typedef struct s_position
{
  int x, y, z;
} position_component;

component_id position_component_id;

typedef struct s_speed
{
  int x, y, z;
} speed_component;
component_id speed_component_id;

typedef struct s_physic
{
  float gravity;
} physic_component;

void update_position(query_it iterator, uint32_t lenght)
{
  position_component *positions = (position_component *)ecs_iterator_get_component_data(iterator, position_component_id);
  speed_component *speeds = (speed_component *)ecs_iterator_get_component_data(iterator, speed_component_id);

  for (size_t i = 0; i < lenght; i++)
  {
    positions[i].x += speeds[i].x;
    positions[i].y += speeds[i].y;
    positions[i].z += speeds[i].z;
  }

  caff_log_info("position: %d %d %d\n", positions[0].x, positions[0].y, positions[0].z);
}

void caffeine_application_setup_world(ecs_world *world)
{
  component_id position_id = ecs_world_add_component(world, "position_component", sizeof(position_component), 8);
  component_id speed_id = ecs_world_add_component(world, "speed_component", sizeof(speed_component), 8);
  component_id physic_id = ecs_world_add_component(world, "physic_component", sizeof(physic_component), 8);
  position_component_id = position_id;
  speed_component_id = speed_id;

  ecs_query_builder *query_builder = ecs_query_builder_new();
  ecs_query_builder_with_component(query_builder, position_id);
  ecs_query_builder_with_component(query_builder, speed_id);

  ecs_query *move_query = ecs_query_builder_build(query_builder);

  ecs_worl_register_system(world, move_query, update_position);

  ecs_archetype runner = ecs_create_archetype(3);
  ecs_archetype_add(&runner, position_id);
  ecs_archetype_add(&runner, speed_id);

  ecs_archetype ball = ecs_create_archetype(3);
  ecs_archetype_add(&ball, position_id);
  ecs_archetype_add(&ball, physic_id);

  archetype_id runner_id = ecs_world_add_archetype(world, runner);

  entity_id e_runner = ecs_world_create_entity(world, runner_id);

  position_component pos = {.x = 5, .y = 7, .z = -1};
  speed_component spd = {.x = 1, .y = 0, .z = -1};
  ecs_world_set_entity_component(world, e_runner, position_id, &pos);
  ecs_world_set_entity_component(world, e_runner, speed_id, &spd);
}
