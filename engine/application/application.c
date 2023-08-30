#include "application.h"
#include "../platform/caffeine_platform.h"
#include "../core/caffeine_events.h"
#include "../core/caffeine_logging.h"

#include "../core/caffeine_logging.h"
#include "../core/caffeine_memory.h"
#include "../core/caffeine_events.h"

typedef struct
{
    char *name;
    bool is_running;
    platform platform;
} application;

static application _application = {0};

void caffeine_application_shutdown();

static bool _caffeine_on_mouse_event(cff_event_data data)
{
    caff_log_info("Mouse position: %d , %d\n", data.mouse_x, data.mouse_y);
    return true;
}

static bool _caffeine_on_quit(cff_event_data data)
{
    _application.is_running = false;
    return true;
}

bool caffeine_application_init(char *app_name)
{
    _application.name = app_name;
    _application.is_running = true;

    cff_memory_init();
    caff_log_init();
    caffeine_event_init();

    if (cff_platform_init(&_application.platform, app_name))
    {
        caffeine_event_register_listener(EVENT_MOUSE_MOVE, _caffeine_on_mouse_event);
        caffeine_event_register_listener(EVENT_QUIT, _caffeine_on_quit);
        return true;
    }

    _application.is_running = false;
    caffeine_event_shutdown();
    caff_log_end();
    cff_memory_end();

    return false;
}

bool caffeine_application_run()
{

    while (_application.is_running)
    {
        cff_platform_poll_events(&_application.platform);
    }

    caffeine_application_shutdown();

    return true;
}


void caffeine_application_shutdown()
{
    caffeine_event_unregister_listener(EVENT_MOUSE_MOVE, _caffeine_on_mouse_event);
    caffeine_event_unregister_listener(EVENT_QUIT, _caffeine_on_quit);

    cff_platform_shutdown(&_application.platform);
    caffeine_event_shutdown();
    caff_log_end();
    cff_memory_end();

    _application = (application){0};
}