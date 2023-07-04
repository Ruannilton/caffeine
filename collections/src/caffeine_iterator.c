#pragma once

#include <caffeine_iterator.h>

uint64_t cff_iterator_count(cff_iterator_s *it)
{
	return it->interf.get_count(it->data);
}

cff_size cff_iterador_data_size(cff_iterator_s *it)
{
	return it->interf.get_data_size(it->data);
}

uint64_t cff_iterator_index(cff_iterator_s *it)
{
	return it->index;
}

void cff_iterator_reset(cff_iterator_s *it)
{
	it->index = 0;
	it->interf.get_item_ref(it->data, it->index, &(it->current_item));
}

bool cff_iterator_next(cff_iterator_s *it)
{
	do
	{
		if (it->index >= it->interf.get_count(it->data))
			return false;
		it->index++;
	} while (!it->interf.get_item_ref(it->data, it->index, &(it->current_item)));

	return true;
}

uintptr_t cff_iterator_current(cff_iterator_s *it)
{
	return it->current_item;
}