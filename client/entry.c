#include <caffeine.h>

typedef struct s_position
{
  int x, y, z;
} position_component;

typedef struct s_speed
{
  int x, y, z;
} speed_component;

typedef struct s_physic
{
  float gravity;
} physic_component;

void update_position(query_it iterator, uint32_t lenght, double delta_time)
{
  if (caff_input_is_key_pressed(KEY_A))
  {

    position_component *positions = (position_component *)ecs_iterator_get_component_data_by_name(iterator, "position_component");
    speed_component *speeds = (speed_component *)ecs_iterator_get_component_data_by_name(iterator, "speed_component");

    for (size_t i = 0; i < lenght; i++)
    {
      positions[i].x += speeds[i].x;
      positions[i].y += speeds[i].y;
      positions[i].z += speeds[i].z;
      caff_log_info("%.8llf, position: %d %d %d\n", delta_time, positions[i].x, positions[i].y, positions[i].z);
    }
  }
}

void register_components(ecs_world *world)
{
  component_id position_id = ecs_world_add_component(world, "position_component", sizeof(position_component), 8);
  component_id speed_id = ecs_world_add_component(world, "speed_component", sizeof(speed_component), 8);
  component_id physic_id = ecs_world_add_component(world, "physic_component", sizeof(physic_component), 8);
}

void register_entities(ecs_world *world)
{
  component_id position_component_id = ecs_world_get_component(world, "position_component");
  component_id speed_component_id = ecs_world_get_component(world, "speed_component");
  component_id physic_component_id = ecs_world_get_component(world, "physic_component");

  ecs_archetype runner = ecs_create_archetype(3);
  ecs_archetype_add(&runner, position_component_id);
  ecs_archetype_add(&runner, speed_component_id);

  ecs_archetype ball = ecs_create_archetype(3);
  ecs_archetype_add(&ball, position_component_id);
  ecs_archetype_add(&ball, physic_component_id);

  archetype_id runner_id = ecs_world_add_archetype(world, runner);
  archetype_id ball_id = ecs_world_add_archetype(world, ball);

  // entity_id e_runner = ecs_world_create_entity(world, runner_id);
  entity_id e_ball = ecs_world_create_entity(world, ball_id);

  position_component pos = {.x = 5, .y = 7, .z = -1};
  speed_component spd = {.x = 1, .y = 0, .z = -1};
  ecs_world_set_entity_component(world, e_ball, position_component_id, &pos);

  ecs_world_add_entity_component(world, e_ball, speed_component_id);
  ecs_world_set_entity_component(world, e_ball, speed_component_id, &spd);
}

void register_systems(ecs_world *world)
{
  component_id position = ecs_world_get_component(world, "position_component");
  component_id speed = ecs_world_get_component(world, "speed_component");

  ecs_query_builder *query_builder = ecs_query_builder_new();

  ecs_query_builder_with_component(query_builder, position);
  ecs_query_builder_with_component(query_builder, speed);

  ecs_query *move_query = ecs_query_builder_build(query_builder);

  ecs_worl_register_system(world, move_query, update_position);
}

void caffeine_application_setup_world(ecs_world *world)
{
  register_components(world);
  register_systems(world);
  register_entities(world);
}
