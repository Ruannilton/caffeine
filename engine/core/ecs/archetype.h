#pragma once

#include "../caffeine_ecs.h"

bool archetype_init(archetype *arch);
bool archetype_init_sized(archetype *arch, uint32_t capacity);
bool archetype_add(archetype *self, component_id component);
bool archetype_remove(archetype *self, component_id component);
bool archetype_compare(archetype *a, archetype *b);
void archetype_release(archetype *arch);
