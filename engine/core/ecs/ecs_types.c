#include "ecs_types.h"
#include "../caffeine_memory.h"

const uint64_t INVALID_ID = (0xffffffff);

ecs_archetype ecs_create_archetype(uint32_t len)
{
    ecs_archetype arch = {
        .components = (component_id *)CFF_ALLOC(len * sizeof(component_id), "ARCHETYPE"),
        .count = 0,
        .capacity = len,
    };
    return arch;
}

void ecs_archetype_add(ecs_archetype *const arch_mut_ref, component_id id)
{
    for (size_t i = 0; i < arch_mut_ref->count; i++)
    {
        if (arch_mut_ref->components[i] == id)
            return;
    }

    if (arch_mut_ref->count == arch_mut_ref->capacity)
    {
        arch_mut_ref->components = CFF_ARR_RESIZE(arch_mut_ref->components, arch_mut_ref->capacity * 2);
        arch_mut_ref->capacity *= 2;
    }

    int i = arch_mut_ref->count - 1;

    while (i >= 0 && arch_mut_ref->components[i] > id)
    {
        arch_mut_ref->components[i + 1] = arch_mut_ref->components[i];
        i--;
    }

    arch_mut_ref->components[i + 1] = id;
    arch_mut_ref->count++;
}
