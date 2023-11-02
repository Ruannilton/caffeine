#include "caffeine_logging.h"
#include "../platform/caffeine_platform.h"
#include "caffeine_memory.h"
#include <stdarg.h>
#include <stdio.h>

#define PRINT_BUFER_LEN 3200

const char *LOG_NAMES[] = {" Error ", " Warn  ", " Debug ", " Info  ",
                           " Trace "};

bool caff_log_init()
{
  caff_log(LOG_LEVEL_TRACE, "Init log system\n");

  caff_log(LOG_LEVEL_TRACE, "Log system initialized\n");
  return true;
}

void caff_log_end()
{
  caff_log(LOG_LEVEL_TRACE, "Shutdown log system\n");

  caff_log(LOG_LEVEL_TRACE, "Log system down\n");
}

void caff_log(log_level level, const char *message, ...)
{

#ifdef CFF_MSVC
  va_list arg_ptr;
#else
  __builtin_va_list arg_ptr;
#endif

  va_start(arg_ptr, message);
  char buffer[PRINT_BUFER_LEN] = {0};
  vsnprintf((char *const)buffer, PRINT_BUFER_LEN, message, arg_ptr);
  va_end(arg_ptr);

  char buffer2[PRINT_BUFER_LEN] = {0};
  sprintf(buffer2, "[%s] %s", LOG_NAMES[level], buffer);

  cff_print_console(level, buffer2);
}

CAFF_API void caff_raw_log(const char *message, ...)
{

#ifdef CFF_MSVC
  va_list arg_ptr;
#else
  __builtin_va_list arg_ptr;
#endif

  va_start(arg_ptr, message);
  char buffer[PRINT_BUFER_LEN] = {0};
  vsnprintf((char *const)buffer, PRINT_BUFER_LEN, message, arg_ptr);
  va_end(arg_ptr);

  char buffer2[PRINT_BUFER_LEN] = {0};
  sprintf(buffer2, "%s", buffer);

  cff_print_console(LOG_LEVEL_DEBUG, buffer2);
}