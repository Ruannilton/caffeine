#include "ecs_types.h"
#include "../caffeine_memory.h"

const uint64_t INVALID_ID = (0xffffffff);

ecs_archetype ecs_create_archetype(uint32_t len)
{
    ecs_archetype arch = {
        .capacity = len,
        .count = 0,
        .components = (component_id *)cff_mem_alloc(len * sizeof(component_id)),
    };
    return arch;
}

void ecs_archetype_add(ecs_archetype *arch, component_id id)
{
    for (size_t i = 0; i < arch->count; i++)
    {
        if (arch->components[i] == id)
            return;
    }

    if (arch->count == arch->capacity)
    {
        arch->components = CFF_ARR_RESIZE(arch->components, arch->capacity * 2);
        arch->capacity *= 2;
    }

    int i = arch->count - 1;

    while (i >= 0 && arch->components[i] > id)
    {
        arch->components[i + 1] = arch->components[i];
        i--;
    }

    arch->components[i + 1] = id;
    arch->count++;
}
