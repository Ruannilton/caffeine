#include "ecs_types.h"
#include "../caffeine_memory.h"
#include "../caffeine_logging.h"

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

void ecs_archetype_add(ecs_archetype *const archetype_mut_ref, component_id id)
{
    for (size_t i = 0; i < archetype_mut_ref->count; i++)
    {
        if (archetype_mut_ref->components[i] == id)
            return;
    }

    if (archetype_mut_ref->count == archetype_mut_ref->capacity)
    {
        archetype_mut_ref->components = CFF_ARR_RESIZE(archetype_mut_ref->components, archetype_mut_ref->capacity * 2);
        archetype_mut_ref->capacity *= 2;
    }

    int i = archetype_mut_ref->count - 1;

    while (i >= 0 && archetype_mut_ref->components[i] > id)
    {
        archetype_mut_ref->components[i + 1] = archetype_mut_ref->components[i];
        i--;
    }

    archetype_mut_ref->components[i + 1] = id;
    archetype_mut_ref->count++;
}

void ecs_archetype_remove(ecs_archetype *const arch_mut_ref, component_id id)
{
    uint32_t i = 0;
    uint32_t found = 0;

    for (i = 0; i < arch_mut_ref->count; i++)
    {
        if (arch_mut_ref->components[i] == id)
        {
            found = 1;
            break;
        }
    }

    if (!found)
    {
        return;
    }

    for (; i < arch_mut_ref->count - 1; i++)
    {
        arch_mut_ref->components[i] = arch_mut_ref->components[i + 1];
    }

    arch_mut_ref->count--;

    if (arch_mut_ref->count < arch_mut_ref->capacity / 2)
    {
        arch_mut_ref->components = CFF_ARR_RESIZE(arch_mut_ref->components, arch_mut_ref->capacity / 2);
        arch_mut_ref->capacity /= 2;
    }
}

ecs_archetype ecs_archetype_copy(const ecs_archetype *const arch_ref)
{
    ecs_archetype arch = {
        .components = (component_id *)CFF_ALLOC(arch_ref->capacity * sizeof(component_id), "ARCHETYPE_COPY"),
        .count = arch_ref->count,
        .capacity = arch_ref->capacity,
    };

    for (size_t i = 0; i < arch_ref->count; i++)
    {
        arch.components[i] = arch_ref->components[i];
    }

    return arch;
}