#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    EVENT_KEY_DOWN,
    EVENT_KEY_UP,
    EVENT_MOUSE_DOWN,
    EVENT_MOUSE_UP,
    EVENT_MOUSE_MOVE,
    EVENT_MOUSE_SCROLL,
    EVENT_QUIT,
    EVENT_RESIZE,

    MAX_EVENTS
} cff_event_code;

typedef struct 
{
    union 
    {
        uint64_t param64[2];
        uint32_t param32[4];
        
        struct {
            uint32_t mouse_x;
            uint32_t mouse_y;
            uint32_t mouse_button;
            uint32_t mouse_scroll;
        };

        struct 
        {
            uint32_t window_width;
            uint32_t window_height;
        };

        struct{
            uint32_t key_code;
            uint32_t key_modifier; 
        };
        
    };
    
}cff_event_data;

typedef bool (*cff_event_callback)(cff_event_data data);

void caffeine_event_init();

void caffeine_event_shutdown();

bool caffeine_event_register_listener(cff_event_code code, cff_event_callback callback);

bool caffeine_event_unregister_listener(cff_event_code code,cff_event_callback callback);

void caffeine_event_fire(cff_event_code code, cff_event_data data);
