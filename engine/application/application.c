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
    return true;
}

static bool _caffeine_on_quit(cff_event_data data)
{
    _application.is_running = false;
    return true;
}

bool caffeine_application_init(char *app_name)
{
    caff_log(LOG_LEVEL_TRACE, "Init application\n");

    _application.name = app_name;
    _application.is_running = true;

    cff_memory_init();

    if (!caff_log_init())
    {
        caff_log(LOG_LEVEL_ERROR, "Failed to initialize log system\n");
        return false;
    }

    caffeine_event_init();

    if (cff_platform_init(&_application.platform, app_name))
    {
        caffeine_event_register_listener(EVENT_MOUSE_MOVE, _caffeine_on_mouse_event);
        caffeine_event_register_listener(EVENT_QUIT, _caffeine_on_quit);

        caff_log(LOG_LEVEL_TRACE, "Application initalized\n");
        return true;
    }

    caff_log(LOG_LEVEL_ERROR, "Failed to initialize platform\n");

    _application.is_running = false;
    caffeine_event_shutdown();
    caff_log_end();
    cff_memory_end();

    return false;
}

bool caffeine_application_run()
{
    caff_log(LOG_LEVEL_TRACE, "Application running\n");

    while (_application.is_running)
    {
        cff_platform_poll_events(&_application.platform);
    }

    caffeine_application_shutdown();

    caff_log(LOG_LEVEL_TRACE, "Application down\n");
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