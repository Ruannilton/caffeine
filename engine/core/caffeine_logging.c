#include "caffeine_logging.h"
#include "../platform/caffeine_platform.h"
#include "caffeine_filesystem.h"
#include "caffeine_memory.h"
#include "caffeine_string.h"
#include <stdarg.h>
#include <stdio.h>

#define PRINT_BUFER_LEN 3200

const char *LOG_NAMES[] = {" Error ", " Warn  ", " Debug ", " Info  ",
                           " Trace "};

// static cff_file cff_log_file_buffer;

bool caff_log_init()
{
  caff_log_terminal(LOG_LEVEL_TRACE, "Init log system\n");

  // path_builder path = cff_path_create_from_app(CFF_GLOBAL_ALLOC);

  // if (IS_ERROR(path.error_code))
  // {
  //   caff_log_terminal(LOG_LEVEL_ERROR, "Failed to build log directory path\n");
  //   return false;
  // }

  // if (IS_ERROR(cff_path_push_file(&path, "log.txt", CFF_GLOBAL_ALLOC)))
  // {
  //   caff_log_terminal(LOG_LEVEL_ERROR, "Failed to build log file path\n");
  //   return false;
  // }

  // // TODO: fix to string
  // cff_string log_path = cff_path_to_string(path, CFF_GLOBAL_ALLOC);
  // caff_log_terminal(LOG_LEVEL_TRACE, "Log file path: %s\n", log_path.buffer);

  // cff_log_file_buffer = cff_file_open_or_create(log_path.buffer, FILE_READ | FILE_WRITE);

  // if (IS_ERROR(cff_log_file_buffer.error))
  // {
  //   caff_log_terminal(LOG_LEVEL_ERROR, "Failed to open log file\n");
  //   return false;
  // }

  // if (IS_ERROR(cff_path_destroy(&path, CFF_GLOBAL_ALLOC)))
  // {
  //   caff_log_terminal(LOG_LEVEL_ERROR, "Failed to deallocate file path\n");
  //   return false;
  // }

  // caff_log_terminal(LOG_LEVEL_INFO, "Log file open at: %s\n", log_path.buffer);

  // cff_string_destroy(&log_path, CFF_GLOBAL_ALLOC);

  caff_log_terminal(LOG_LEVEL_TRACE, "Log system initialized\n");
  return true;
}

void caff_log_end()
{
  caff_log(LOG_LEVEL_TRACE, "Shutdown log system\n");

  // if (cff_log_file_buffer.open)
  // {
  //   if (IS_ERROR(cff_file_close(&cff_log_file_buffer)))
  //   {
  //     caff_log_terminal(LOG_LEVEL_ERROR, "Failed to close log file\n");
  //   }
  // }

  caff_log_terminal(LOG_LEVEL_TRACE, "Log system down\n");
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

  // uint64_t buffer2_len = strlen(buffer2);
  // cff_file_write_line(&cff_log_file_buffer, buffer2, buffer2_len);

  cff_print_console(level, buffer2);
}

void caff_log_terminal(log_level level, const char *message, ...)
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