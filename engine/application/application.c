#include "application.h"
#include "../platform/caffeine_platform.h"
#include "../core/caffeine_events.h"
#include "../core/caffeine_logging.h"

#include "../core/caffeine_logging.h"
#include "../core/caffeine_memory.h"
#include "../core/caffeine_events.h"
#include "../core/caffeine_input.h"

typedef struct
{
    char *name;
    bool is_running;
    bool is_paused;
} application;

static application _application = {0};

void caffeine_application_shutdown();

static bool _mouse_move(cff_event_data data)
{
    caff_log(LOG_LEVEL_TRACE, "Mouse Move: %d, %d\n", data.mouse_x, data.mouse_y);
    return true;
}

static bool _key_press(cff_event_data data)
{
    caff_log(LOG_LEVEL_TRACE, "Key Press: %c\n", data.key_code);
    return true;
}

static bool _key_release(cff_event_data data)
{
    caff_log(LOG_LEVEL_TRACE, "Key Release: %c\n", data.key_code);
    return true;
}

static void _caffeine_on_quit()
{
    caff_log(LOG_LEVEL_TRACE, "Quit application\n");
    caffeine_event_fire(EVENT_QUIT, (cff_event_data){0});
    _application.is_running = false;
}

bool caffeine_application_init(char *app_name)
{
    caff_log(LOG_LEVEL_TRACE, "Init application\n");

    _application.name = app_name;
    _application.is_running = true;
    _application.is_paused = false;

    cff_memory_init();

    if (!caff_log_init())
    {
        caff_log(LOG_LEVEL_ERROR, "Failed to initialize log system\n");
        return false;
    }

    caffeine_event_init();

    caff_input_init();

    if (cff_platform_init(app_name))
    {
        cff_platform_set_quit_clkb(_caffeine_on_quit);

        caffeine_event_register_listener(EVENT_KEY_DOWN, _key_press);
        caffeine_event_register_listener(EVENT_KEY_UP, _key_release);
        caffeine_event_register_listener(EVENT_MOUSE_MOVE, _mouse_move);

        caff_log(LOG_LEVEL_TRACE, "Application initalized\n");
        return true;
    }

    caff_log(LOG_LEVEL_ERROR, "Failed to initialize platform\n");

    caffeine_application_shutdown();

    return false;
}

bool caffeine_application_run()
{
    caff_log(LOG_LEVEL_TRACE, "Application running\n");

    while (_application.is_running)
    {
        cff_platform_poll_events();

        if (_application.is_paused)
            continue;

        caff_input_update();
    }

    caffeine_application_shutdown();

    caff_log(LOG_LEVEL_TRACE, "Application down\n");
    return true;
}

void caffeine_application_shutdown()
{
    _application.is_running = false;
    _application.is_paused = true;

    cff_platform_shutdown();

    caff_input_end();
    caffeine_event_shutdown();
    caff_log_end();
    cff_memory_end();

    _application = (application){0};
}