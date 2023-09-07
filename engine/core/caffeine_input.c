#include "caffeine_input.h"
#include "caffeine_logging.h"
#include "caffeine_events.h"
#include "../platform/caffeine_platform.h"

typedef struct keyboard_state
{
    button_state keys[KEYS_MAX_KEYS];
} keyboard_state;

typedef struct mouse_state
{
    uint32_t x_pos, y_pos;
    button_state buttons[BUTTON_MAX_BUTTONS];
    int32_t scroll;
} mouse_state;

static mouse_state
    mouse_prev_state,
    mouse_current_state;

static keyboard_state
    keyboard_prev_state,
    keyboard_current_state;

void input_key_clkb(uint32_t key, uint32_t state)
{
    if ((keys)key < KEYS_MAX_KEYS)
    {
        state = (button_state)state;
        if (keyboard_current_state.keys[key] != state)
        {
            cff_event_data data = {.key_code = (keys)key};
            cff_event_code code = state == PRESSED ? EVENT_KEY_DOWN : EVENT_KEY_UP;
            caffeine_event_fire(code, data);
            keyboard_current_state.keys[key] = state;
        }
    }
}

void input_mouse_button_clkb(uint32_t button, uint32_t state)
{
    if ((buttons)button < BUTTON_MAX_BUTTONS)
    {
        mouse_current_state.buttons[button] = (button_state)state;

        state = (button_state)state;
        if (mouse_current_state.buttons[button] != state)
        {
            cff_event_data data = {.mouse_button = (buttons)button};
            cff_event_code code = state == PRESSED ? EVENT_MOUSE_DOWN : EVENT_MOUSE_UP;
            caffeine_event_fire(code, data);
            mouse_current_state.buttons[button] = state;
        }
    }
}

void input_mouse_move_clkb(uint32_t x, uint32_t y)
{
    if (mouse_current_state.x_pos != x || mouse_current_state.y_pos != y)
    {
        cff_event_data data = {.mouse_x = x, .mouse_y = y};
        caffeine_event_fire(EVENT_MOUSE_MOVE, data);
        mouse_current_state.x_pos = x;
        mouse_current_state.y_pos = y;
    }
}

void input_mouse_scroll_clkb(int32_t scroll)
{
    cff_event_data data = {.mouse_scroll = scroll};
    caffeine_event_fire(EVENT_MOUSE_SCROLL, data);
    mouse_current_state.scroll = scroll;
}

void caff_input_init(void)
{
    caff_log(LOG_LEVEL_TRACE, "Init input system\n");

    cff_platform_set_key_clbk(input_key_clkb);
    cff_platform_set_mouse_button_clkb(input_mouse_button_clkb);
    cff_platform_set_mouse_move_clkb(input_mouse_move_clkb);
    cff_platform_set_mouse_scroll_clkb(input_mouse_scroll_clkb);
}

void caff_input_end(void)
{
    caff_log(LOG_LEVEL_TRACE, "Shutdown input system\n");
}

void caff_input_update(void)
{
    mouse_prev_state = mouse_current_state;
    keyboard_prev_state = keyboard_current_state;
}
