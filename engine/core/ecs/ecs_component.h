#pragma once

#include "ecs_types.h"

/*
 estrutura para armazenar os dados dos componentes registrados
 internamente é um array de objetos contendo o id, nome, tamanho e alinhamento do componente
 o id do componente é a posição dele no array
*/
typedef struct component_index component_index;

component_index *ecs_new_component_index(uint32_t capacity);
component_id ecs_register_component(component_index *index, const char *name, size_t size, size_t align);
size_t ecs_get_component_size(component_index *index, component_id id);
size_t ecs_get_component_align(component_index *index, component_id id);
void ecs_remove_component(component_index *index, component_id id);
void ecs_release_component_index(component_index *index);