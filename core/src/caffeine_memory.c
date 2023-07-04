#include <caffeine_memory.h>
#include <caffeine_flags.h>
#include <caffeine_macros.h>
#include <caffeine_macros.h>
#include <malloc.h>

static inline void internal_cff_mem_copy(const void* from, void* dest, cff_size size) {
	register const uintptr_t* f = (const uintptr_t*)(from);
	register uintptr_t* d = (uintptr_t*)(dest);

	while (size >= sizeof(uintptr_t))
	{
		*(d++) = *(f++);
		size -= sizeof(uintptr_t);
	}

	register const uint8_t* char_f = (const uint8_t*)f;
	register uint8_t* char_d = (uint8_t*)d;

	while (size)
	{
		*(char_d++) = *(char_f++);
		size -= sizeof(uint8_t);
	}
}

void* cff_malloc(cff_size size) {
	return malloc((size_t)size);
}

void* cff_stack_alloc(cff_size size) {
#ifdef CFF_COMP_MSVC
	return _malloca((size_t)size);
#elif CFF_COMP_GCC
	return alloca(size);
#endif

}

void* cff_realloc(void* ptr, cff_size size) {
	debug_assert(ptr != NULL);
	return realloc(ptr, size);
}

void cff_free(void* ptr) {
	debug_assert(ptr != NULL);
	free(ptr);
}

void cff_stack_free(void* ptr) {
	debug_assert(ptr != NULL);

#ifdef CFF_COMP_MSVC
	_freea(ptr);
#elif CFF_COMP_GCC

#endif
}

cff_size cff_get_size(void* ptr) {
	debug_assert(ptr != NULL);

#ifdef CFF_COMP_MSVC
	return (cff_size)_msize(ptr);
#else
	return (cff_size)(0 / 0);
#endif
}

void cff_mem_copy(const void* from, void* dest, cff_size size) {

	if (from == dest || size == 0)
		return;

	internal_cff_mem_copy(from, dest, size);
}

void cff_mem_move(const void* from, void* dest, cff_size size) {
	register const uint8_t* f = (const uint8_t*)from;
	register uint8_t* d = (uint8_t*)dest;

	if (f == d || size == 0)
		return;

	if (d > f && ((unsigned int)(d - f)) < size) {
		int i;
		for (i = size - 1; i >= 0; i--)
			d[i] = f[i];
		return;
	}

	if (f > d && ((unsigned int)(f - d)) < size) {
		size_t i;
		for (i = 0; i < size; i++)
			d[i] = f[i];
		return;
	}

	cff_mem_copy(from, dest, size);
}

void cff_mem_set(const void* data, void* dest, cff_size data_size, cff_size buffer_lenght) {
	debug_assert(buffer_lenght % data_size == 0);

	uintptr_t dest_start = (uintptr_t)dest;

	for (cff_size i = 0; i < buffer_lenght; i += data_size)
	{
		void* address = (void*)(dest_start + i);
		internal_cff_mem_copy(data, address, data_size);
	}
}

bool cff_mem_cmp(const void* const a, const void* const b, cff_size size) {
	register const uintptr_t* f = (const uintptr_t*)(a);
	register const uintptr_t* d = (const uintptr_t*)(b);

	if (a == b || size == 0)
		return true;

	while (size >= sizeof(uintptr_t))
	{
		if (*(d++) != *(f++)) return false;
		size -= sizeof(uintptr_t);
	}

	register const uint8_t* char_f = (const uint8_t*)f;
	register const uint8_t* char_d = (const uint8_t*)d;

	while (size)
	{
		if (*(char_d++) != *(char_f++)) return false;
		size -= sizeof(uint8_t);
	}

	return true;
}