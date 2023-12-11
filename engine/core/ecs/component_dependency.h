#pragma once

#include "ecs_types.h"
#include "../ds/caffeine_vector.h"
#include "../ds/caffeine_hashmap.h"

cff_arr_dcltype(dependency_list, archetype_id);
cff_hash_dcltype(component_dependency, component_id, dependency_list);

component_dependency *ecs_component_dependency_init(uint32_t capacity);
void ecs_component_dependency_release(const component_dependency *const ptr_owning);
void ecs_component_dependency_remove_component(component_dependency *const ptr_mut_ref, component_id component);
void ecs_component_dependency_add_dependency(component_dependency *const ptr_mut_ref, component_id component, archetype_id archetype);
void ecs_component_dependency_remove_dependency(component_dependency *const ptr_mut_ref, component_id component, archetype_id archetype);
uint32_t ecs_component_dependency_get_dependencies(const component_dependency *const ptr_ref, component_id component, const archetype_id **out_mut_ref);
component_id ecs_component_dependency_get_less_dependencies(const component_dependency *const ptr_ref, const component_id *const components_ref, uint32_t len);
component_id ecs_component_dependency_get_max_dependencies(const component_dependency *const ptr_ref, const component_id *const components_ref, uint32_t len);