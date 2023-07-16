#include "caffeine_logging.h"
#include "caffeine_platform.h"

#include <stdarg.h>
#include <stdio.h>

#define PRINT_BUFER_LEN 3200

const char *LOG_NAMES[] = {" Error ", " Warning", " Debug ", " Info  ",
                           " Trace "};

void caff_log_init() {}

void caff_log_end() {}

void caff_log(log_level level, const char *message, ...) {

#ifdef CFF_MSVC
  va_list arg_ptr;
#else
  __builtin_va_list arg_ptr;
#endif

  va_start(arg_ptr, message);
  char buffer[PRINT_BUFER_LEN] = {0};
  vsnprintf((char *const)buffer, PRINT_BUFER_LEN, message, arg_ptr);
  va_end(arg_ptr);

  cff_print_console("[%s] %s", LOG_NAMES[level], buffer);
}