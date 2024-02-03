#include "ecs_name_index.h"

cff_hash_impl(name_index, caff_string, uint64_t);

uint32_t hash_caff_string(caff_string *name, uint32_t c)
{
    const char *str_name = *name;
    size_t lenght = strnlen(str_name, MAX_NAME_LENGHT);
    uint32_t hash = c;
    const uint8_t *byte_data = (const uint8_t *)str_name;
    for (size_t i = 0; i < lenght; i++)
    {
        hash = (hash * 31) + byte_data[i];
    }

    return hash;
}

bool cmp_caff_string(caff_string *a, caff_string *b)
{
    const char *str_a = *a;
    const char *str_b = *b;
    return strncmp(str_a, str_b, MAX_NAME_LENGHT) == 0;
}

bool cmp_id(uint64_t *a, uint64_t *b)
{
    return *a == *b;
}

void ecs_name_index_init(name_index *index)
{
    name_index_init(index, 8, hash_caff_string, cmp_caff_string, cmp_id);
}

uint8_t ecs_name_index_get(const name_index *index, caff_string name, uint64_t *out)
{
    if (out == NULL)
        return false;
    *out = INVALID_ID;
    if (name_index_get(index, name, out))
    {
        return 1;
    }
    return 0;
}

bool ecs_name_index_remove(name_index *index, caff_string name)
{
    return name_index_remove(index, name);
}

void ecs_name_index_add(name_index *index, caff_string name, uint64_t id)
{
    name_index_add(index, name, id);
}

void ecs_name_index_release(const name_index *index)
{
    name_index_release(index);
}