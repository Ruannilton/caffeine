#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum button_state
{
    RELEASED = 0,
    PRESSED = 1,
} button_state;

void caff_input_init(void);
void caff_input_end(void);
void caff_input_update(void);
