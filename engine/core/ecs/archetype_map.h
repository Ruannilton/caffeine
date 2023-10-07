#pragma once

#include "../caffeine_ecs.h"
#include "archetype.h"

bool map_init(uint32_t capacity);
void map_release();
bool map_add(archetype value, archetype_id *out);
archetype *map_get(archetype_id id);
bool map_build();