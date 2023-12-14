#include "../caffeine_types.h"
#include "../core/ecs/ecs_world.h"

CAFF_API bool caffeine_application_init(char *app_name, void (*ecs_init)(ecs_world *));
CAFF_API bool caffeine_application_run();
