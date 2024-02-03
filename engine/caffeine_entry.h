#pragma once

#include "./core/ecs/ecs_world.h"
#include "application/application.h"

extern void caffeine_application_setup_world(ecs_world *world);

int main(void)
{
    bool app_started = caffeine_application_init("Caffeine", caffeine_application_setup_world);

    if (!app_started)
        return 1;

    bool sucess_exit = caffeine_application_run();

    return sucess_exit ? 0 : 2;
}