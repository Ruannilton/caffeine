#include "../core/ecs/ecs_world.h"
#include "../core/caffeine_logging.h"
#include <inttypes.h>
#include <stdio.h>

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

void ecs_setup_test()
{
    ecs_world *world = ecs_world_new();

    component_id position_id = ecs_world_add_component(world, "position_component", sizeof(position_component), 8);
    component_id speed_id = ecs_world_add_component(world, "speed_component", sizeof(speed_component), 8);
    component_id physic_id = ecs_world_add_component(world, "physic_component", sizeof(physic_component), 8);

    ecs_archetype runner = ecs_create_archetype(3);
    ecs_archetype_add(&runner, position_id);
    ecs_archetype_add(&runner, speed_id);

    ecs_archetype ball = ecs_create_archetype(3);
    ecs_archetype_add(&ball, position_id);
    ecs_archetype_add(&ball, physic_id);

    archetype_id runner_id = ecs_world_add_archetype(world, runner);
    archetype_id ball_id = ecs_world_add_archetype(world, ball);

    entity_id e_runner = ecs_world_create_entity(world, runner_id);
    entity_id e_ball = ecs_world_create_entity(world, ball_id);

    position_component pos = {.x = 5, .y = 7, .z = -1};
    ecs_world_set_entity_component(world, e_runner, position_id, &pos);

    position_component *result = ecs_world_get_entity_component(world, e_runner, position_id);

    printf("%d, %d, %d\n", result->x, result->y, result->z);

    (void)runner_id;
    (void)ball_id;
    (void)e_runner;
    (void)e_ball;

    ecs_world_release(world);
}