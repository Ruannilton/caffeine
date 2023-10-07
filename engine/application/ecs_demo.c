#include "../core/caffeine_ecs.h"
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
    component_id position_comp = caff_ecs_add_register_component(sizeof(position_component));
    if (position_comp == INVALID_COMPONENT_ID)
        caff_log(LOG_LEVEL_ERROR, "Failed to register position component");
    else
        caff_log(LOG_LEVEL_TRACE, "Position Component registered with Id: %u\n", position_comp);

    component_id speed_comp = caff_ecs_add_register_component(sizeof(speed_component));
    if (speed_comp == INVALID_COMPONENT_ID)
        caff_log(LOG_LEVEL_ERROR, "Failed to register speed component");
    else
        caff_log(LOG_LEVEL_TRACE, "Speed Component registered with Id: %u\n", speed_comp);

    archetype_id runner_id = caff_ecs_archetype_new(2, position_comp, speed_comp);

    if (runner_id == INVALID_ARCHETYPE_ID)
        caff_log(LOG_LEVEL_ERROR, "Failed to register runner archetype");
    else
        caff_log(LOG_LEVEL_TRACE, "Runner archetype registered with Id: %u\n", runner_id);

    caff_ecs_build_world();

    entity_id e_runner_id = caff_ecs_entity_new(runner_id);

    if (e_runner_id == INVALID_ENTITY_ID)
        caff_log(LOG_LEVEL_ERROR, "Failed to create runner entity");
    else
        caff_log(LOG_LEVEL_TRACE, "Runner entity registered with Id: %" PRIu64 "\n", e_runner_id);

    position_component *runner_position = (position_component *)caff_ecs_entity_get_component(e_runner_id, position_comp);

    if (runner_position == NULL)
        caff_log(LOG_LEVEL_ERROR, "Failed to get position component");

    runner_position->x = 5;
    runner_position->y = 234;
    runner_position->z = -4;

    position_component *runner_position_2 = (position_component *)caff_ecs_entity_get_component(e_runner_id, position_comp);

    caff_log(LOG_LEVEL_TRACE, "Position Component: %d %d %d\n", runner_position_2->x, runner_position_2->y, runner_position_2->z);
}