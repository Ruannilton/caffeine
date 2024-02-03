#pragma once

#include <stdint.h>

void caff_time_tick(void);
double caff_time_current(void);
double caff_time_delta(void);
void caff_time_sleep(uint64_t ms);