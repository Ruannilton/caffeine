#include "caffeine_input.h"

#include "caffeine_logging.h"

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

void caff_input_init(void)
{
    caff_log(LOG_LEVEL_TRACE, "Init input system\n");
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

void caff_input_set_key(keys key, button_state state)
{
    if (key < KEYS_MAX_KEYS)
    {
        keyboard_current_state.keys[key] = state;
    }
}

void caff_input_set_mouse_button(buttons button, button_state state)
{
    if (button < BUTTON_MAX_BUTTONS)
    {
        mouse_current_state.buttons[button] = state;
    }
}

void caff_input_set_mouse_position(uint32_t x, uint32_t y)
{
    mouse_current_state.x_pos = x;
    mouse_current_state.y_pos = y;
}

void caff_input_set_mouse_scroll(int32_t scroll)
{
    mouse_current_state.scroll = scroll;
}