#pragma once

#include "../caffeine_types.h"
#include <string.h>

bool caff_log_init();
void caff_log_end();

#define caff_log_error(message, ...) \
  caff_log(LOG_LEVEL_ERROR, message, __VA_ARGS__)

#define caff_log_warn(message, ...) \
  caff_log(LOG_LEVEL_WARNING, message, __VA_ARGS__)

#define caff_log_debug(message, ...) \
  caff_log(LOG_LEVEL_DEBUG, message, __VA_ARGS__)

#define caff_log_info(message, ...) \
  caff_log(LOG_LEVEL_INFO, message, __VA_ARGS__)

#define caff_log_trace(message, ...) \
  caff_log(LOG_LEVEL_TRACE, message, __VA_ARGS__)

#define caff_raw_log(message, ...) \
  caff_raw_log(message, __VA_ARGS__)

CAFF_API void caff_log(log_level level, const char *message, ...);
CAFF_API void caff_raw_log(const char *message, ...);
