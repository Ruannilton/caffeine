
#include <time.h>

#include "../platform/caffeine_platform.h"
#include "caffeine_time.h"

static double current_time = 0, last_time = 0, delta_time;

void caff_time_tick(void)
{
    current_time = clock() / (double)CLOCKS_PER_SEC;
    delta_time = current_time - last_time;
    last_time = current_time;
}

double caff_time_current(void)
{
    return current_time;
}

double caff_time_delta(void)
{
    return delta_time;
}

void caff_time_sleep(uint64_t ms)
{
    cff_platform_sleep(ms);
}