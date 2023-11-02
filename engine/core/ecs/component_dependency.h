#pragma once

#include "ecs_types.h"
#include "../ds/caffeine_vector.h"

cff_arr_dcltype(dependency_list, archetype_id);
cff_arr_dcltype(component_dependency, dependency_list);

component_dependency *ecs_component_dependency_init(uint32_t capacity);
void ecs_component_dependency_release(component_dependency *ptr);
void ecs_component_dependency_remove_component(component_dependency *ptr, component_id component);
void ecs_component_dependency_add_dependency(component_dependency *ptr, component_id component, archetype_id archetype);
void ecs_component_dependency_remove_dependency(component_dependency *ptr, component_id component, archetype_id archetype);
uint32_t ecs_component_dependency_get_dependencies(component_dependency *ptr, component_id component, const archetype_id *out);
component_id ecs_component_dependency_get_less_dependencies(component_dependency *ptr, const component_id *components, uint32_t len);
component_id ecs_component_dependency_get_max_dependencies(component_dependency *ptr, const component_id *components, uint32_t len);