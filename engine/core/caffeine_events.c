#include "caffeine_events.h"

#define EVENT_LISTENER_MAX 64

typedef struct
{
    cff_event_callback callbacks[EVENT_LISTENER_MAX];
    uint32_t count;
} listener_entry;

static listener_entry _listeners[MAX_EVENTS];

void caffeine_event_init()
{

    for (size_t i = 0; i < MAX_EVENTS; i++)
    {
        _listeners[i].count = 0;
    }
}

void caffeine_event_shutdown()
{
    for (size_t i = 0; i < MAX_EVENTS; i++)
    {
        _listeners[i].count = 0;
    }
}

bool caffeine_event_register_listener(cff_event_code code, cff_event_callback callback)
{
    uint32_t index = _listeners[code].count;
    if (index == EVENT_LISTENER_MAX)
        return false;
    _listeners[code].callbacks[index] = callback;
    _listeners[code].count++;
    return true;
}

bool caffeine_event_unregister_listener(cff_event_code code, cff_event_callback callback)
{
    cff_event_callback *listeners = _listeners[code].callbacks;
    uint32_t listener_count = _listeners[code].count;

    for (uint32_t c = 0; c < listener_count; c++)
    {
        if (listeners[c] == callback)
        {
            listeners[c] = listeners[_listeners[code].count - 1];
            _listeners[code].count--;
            return true;
        }
    }
    return false;
}

void caffeine_event_fire(cff_event_code code, cff_event_data data)
{
    uint32_t listener_count = _listeners[code].count;
    cff_event_callback *listeners = _listeners[code].callbacks;

    for (uint32_t c = 0; c < listener_count; c++)
        if(listeners[c](data)) break;

}
