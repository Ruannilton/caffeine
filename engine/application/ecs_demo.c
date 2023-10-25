#include "../core/ecs/caffeine_ecs.h"
#include "../core/caffeine_logging.h"
#include <inttypes.h>

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

void move(query_it *it)
{
    position_component *positions = ecs_query_it_get_components(it, 0);
    speed_component *speeds = ecs_query_it_get_components(it, 1);
    uint32_t count = ecs_query_it_count(it);

    for (size_t i = 0; i < count; i++)
    {
        positions[i].x += speeds[i].x;
        positions[i].y += speeds[i].y;
        positions[i].z += speeds[i].z;
    }
}

void grav(query_it *it)
{
    position_component *positions = ecs_query_it_get_components(it, 0);
    physic_component *physics = ecs_query_it_get_components(it, 2);
    uint32_t count = ecs_query_it_count(it);

    for (size_t i = 0; i < count; i++)
    {
        positions[i].y += physics[i].gravity;
    }
}

void ecs_setup_test()
{
    component_id position = ecs_register_component("position_component", sizeof(position_component));
    component_id speed = ecs_register_component("speed_component", sizeof(speed_component));
    component_id physic = ecs_register_component("physic_component", sizeof(physic_component));

    archetype runner_arch;
    cff_arr_init(&runner_arch, 2);
    cff_arr_add(&runner_arch, position);
    cff_arr_add(&runner_arch, speed);

    archetype runner_arch_cpy;
    cff_arr_init(&runner_arch_cpy, 2);
    cff_arr_add(&runner_arch_cpy, position);
    cff_arr_add(&runner_arch_cpy, speed);

    archetype bullet_arch;
    cff_arr_init(&bullet_arch, 3);
    cff_arr_add(&bullet_arch, position);
    cff_arr_add(&bullet_arch, speed);
    cff_arr_add(&bullet_arch, physic);

    archetype ball_arch;
    cff_arr_init(&ball_arch, 2);
    cff_arr_add(&ball_arch, position);
    cff_arr_add(&ball_arch, physic);

    archetype ball_arch_cpy;
    cff_arr_init(&ball_arch_cpy, 2);
    cff_arr_add(&ball_arch_cpy, position);
    cff_arr_add(&ball_arch_cpy, physic);

    ecs_query query_position_speed = {
        .exact = false,
        .with_components = runner_arch_cpy,
    };

    query_id q_position_speed = ecs_register_query(query_position_speed);

    archtype_id a_runner = ecs_register_archetype(runner_arch);
    archtype_id a_bullet = ecs_register_archetype(bullet_arch);
    archtype_id a_ball = ecs_register_archetype(ball_arch);

    ecs_query query_position_pysic = {
        .exact = false,
        .with_components = ball_arch_cpy,
    };

    query_id q_position_pysic = ecs_register_query(query_position_pysic);

    entity_id e_runner = ecs_create_entity(a_runner);
    entity_id e_bullet = ecs_create_entity(a_bullet);
    entity_id e_ball = ecs_create_entity(a_ball);

    ecs_register_system(q_position_speed, move);
    ecs_register_system(q_position_pysic, grav);

    {
        // Runner
        position_component *runner_pos = (position_component *)ecs_get_entity_component(e_runner, position);
        *runner_pos = (position_component){.x = 0, .y = 0, .z = 0};
        speed_component *runner_speed = (speed_component *)ecs_get_entity_component(e_runner, speed);
        *runner_speed = (speed_component){.x = 1, .y = 0, .z = 0};

        // Bullet
        position_component *bullet_pos = (position_component *)ecs_get_entity_component(e_bullet, position);
        *bullet_pos = (position_component){.x = 0, .y = 0, .z = 0};
        speed_component *bullet_speed = (speed_component *)ecs_get_entity_component(e_bullet, speed);
        *bullet_speed = (speed_component){.x = 1, .y = 0, .z = 0};
        physic_component *bullet_physic = (physic_component *)ecs_get_entity_component(e_bullet, physic);
        *bullet_physic = (physic_component){.gravity = -9.8f};

        // Ball
        position_component *buall_pos = (position_component *)ecs_get_entity_component(e_ball, position);
        *buall_pos = (position_component){.x = 0, .y = 0, .z = 0};
        physic_component *ball_physic = (physic_component *)ecs_get_entity_component(e_ball, physic);
        *ball_physic = (physic_component){.gravity = -9.8f};
    }

    ecs_update();
    ecs_update();
    ecs_update();

    {
        // Runner
        position_component *runner_pos = (position_component *)ecs_get_entity_component(e_runner, position);
        speed_component *runner_speed = (speed_component *)ecs_get_entity_component(e_runner, speed);

        // Bullet
        position_component *bullet_pos = (position_component *)ecs_get_entity_component(e_bullet, position);
        speed_component *bullet_speed = (speed_component *)ecs_get_entity_component(e_bullet, speed);
        physic_component *bullet_physic = (physic_component *)ecs_get_entity_component(e_bullet, physic);

        // Ball
        position_component *buall_pos = (position_component *)ecs_get_entity_component(e_ball, position);
        physic_component *ball_physic = (physic_component *)ecs_get_entity_component(e_ball, physic);

        (void)runner_pos;
        (void)runner_speed;
        (void)buall_pos;
        (void)ball_physic;
        (void)bullet_pos;
        (void)bullet_speed;
        (void)bullet_physic;
    }
}