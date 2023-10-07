#pragma once

#include "../caffeine_ecs.h"

bool component_list_init();
component_id component_list_add(uint32_t size);
uint32_t get_component_size(component_id id);
void component_list_end();