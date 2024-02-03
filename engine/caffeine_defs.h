#pragma once

#include "caffeine_flags.h"

#ifdef CAFF_EXPORT
#ifdef CFF_MSVC
#define CAFF_API __declspec(dllexport)
#else
#define CAFF_API __attribute__((visibility("default")))
#endif
#else
#ifdef CFF_MSVC
#define CAFF_API __declspec(dllimport)
#else
#define CAFF_API
#endif
#endif

#ifdef DEBUG
#define debug_assert(x) assert(x)
#define release_assert(x) assert(x)
#else
#define debug_assert(x)
#define release_assert(x) assert(x)
#define CFF_DEBUG
#endif

#define ADDRESS_SIZE_BYTES (sizeof(void *))

#define ADDRESS_SIZE_BITS (ADDRESS_SIZE_BYTES * 8)

#define comptime_assert(x, message) _Static_assert(x, message)

#define ____concat2(a, b) a##b

#define concat_token(a, b) ____concat2(a, b)

#define macro_var(name) concat_token(name, __LINE__)

#define defer(init, end)                                                \
  for (int macro_var(__defer_i_) = ((init), 0); !macro_var(__defer_i_); \
       (macro_var(__defer_i_) += 1), (end))

#define scope(action) defer(0, action)

#ifdef CFF_DEBUG
#define CFF_DEBUG_BLOCK(CONTENT) CONTENT
#else
#define CFF_DEBUG_BLOCK(CONTENT)
#endif