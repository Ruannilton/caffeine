#include "archetype.h"

#define LOG_SCOPE "ECS::Archetype - "

static bool _archetype_resize(archetype *arch)
{
    uint32_t new_capacity = arch->capacity * 2;
    arch->buffer = cff_resize_arr(arch->buffer, new_capacity);

    if (arch->buffer == NULL)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to resize\n");
        return false;
    }
    else
    {
        arch->capacity = new_capacity;
    }
    return true;
}

bool archetype_init(archetype *arch)
{
    arch->capacity = 4;
    arch->count = 0;
    arch->buffer = cff_new_arr(component_id, arch->capacity);

    if (arch->buffer == NULL)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to init\n");
        return false;
    }

    return true;
}

bool archetype_init_sized(archetype *arch, uint32_t capacity)
{
    arch->capacity = capacity;
    arch->count = 0;
    arch->buffer = cff_new_arr(component_id, arch->capacity);

    if (arch->buffer == NULL)
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to init\n");
        return false;
    }

    return true;
}
// TODO: optimize with binary search

bool archetype_add(archetype *self, component_id component)
{
    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Adding component: %d\n", component);

    if (self->count == self->capacity)
        if (!_archetype_resize(self))
        {
            caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to add component: %d\n", component);
            return false;
        }

    uint32_t i;
    for (i = 0; i < self->count && self->buffer[i] < component; i++)
        ;

    for (uint32_t j = self->count; j > i; j--)
    {
        self->buffer[j] = self->buffer[j - 1];
    }

    self->buffer[i] = component;
    self->count++;

    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Component: %d added\n", component);
    return true;
}

bool archetype_remove(archetype *self, component_id component)
{
    uint32_t i, j;

    for (i = 0; i < self->count && self->buffer[i] != component; i++)
        ;

    if (i < self->count)
    {
        for (j = i; j < self->count - 1; j++)
        {
            self->buffer[j] = self->buffer[j + 1];
        }
        self->count--;
    }
    else
    {
        caff_log(LOG_LEVEL_ERROR, LOG_SCOPE "Failed to remove component: %d\n", component);
        return false;
    }
    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Component: %d removed\n", component);
    return true;
}

void archetype_release(archetype *arch)
{
    if (arch && arch->buffer)
    {
        cff_mem_release((void *)arch->buffer);
    }
    *arch = (archetype){0};

    caff_log(LOG_LEVEL_TRACE, LOG_SCOPE "Released\n");
}

bool archetype_compare(archetype *a, archetype *b)
{
    if (a->count != b->count)
        return false;

    for (uint32_t i = 0; i < a->count; i++)
    {
        if (a->buffer[i] != b->buffer[i])
            return false;
    }

    return true;
}
