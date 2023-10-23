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

void ecs_setup_test()
{
    component_id position = ecs_register_component("position_component", sizeof(position_component));
    component_id speed = ecs_register_component("speed_component", sizeof(speed_component));

    archetype runner_arch;
    cff_arr_init(&runner_arch, 2);
    cff_arr_add(&runner_arch, position);
    cff_arr_add(&runner_arch, speed);

    archtype_id runner = ecs_register_archetype(runner_arch);

    entity_id ent = ecs_create_entity(runner);

    position_component *ent_pos = (position_component *)ecs_get_entity_component(ent, position);
    *ent_pos = (position_component){.x = 5, .y = 0, .z = -7};

    speed_component *ent_spd = (speed_component *)ecs_get_entity_component(ent, speed);
    *ent_spd = (speed_component){.x = 0, .y = 0, .z = 1};

    position_component *ent_pos_2 = (position_component *)ecs_get_entity_component(ent, position);
    speed_component *ent_spd_2 = (speed_component *)ecs_get_entity_component(ent, speed);

    (void)ent_pos_2;
    (void)ent_spd_2;
}