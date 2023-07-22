#include "caffeine_logging.h"
#include "caffeine_filesystem.h"
#include "caffeine_memory.h"
#include "caffeine_platform.h"
#include "caffeine_string.h"
#include <stdarg.h>
#include <stdio.h>

#define PRINT_BUFER_LEN 3200

const char *LOG_NAMES[] = {" Error ", " Warning", " Debug ", " Info  ",
                           " Trace "};

static cff_file cff_log_file_buffer;

void caff_log_init() {
  path_builder path = cff_path_create_from_app(cff_global_allocator);
  cff_path_push_file(&path, "log.txt", cff_global_allocator);

  // TODO: fix to string
  cff_string log_path = cff_path_to_string(path, cff_global_allocator);

  cff_log_file_buffer =
      cff_file_open_or_create(log_path.buffer, FILE_READ | FILE_WRITE);

  cff_path_destroy(&path, cff_global_allocator);
  cff_string_destroy(&log_path, cff_global_allocator);
}

void caff_log_end() {
  if (cff_log_file_buffer.open) {
    cff_file_close(&cff_log_file_buffer);
  }
}

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

  char buffer2[PRINT_BUFER_LEN] = {0};
  sprintf(buffer2, "[%s] %s", LOG_NAMES[level], buffer);
  uint64_t buffer2_len = strlen(buffer2);

  cff_file_write_line(&cff_log_file_buffer, buffer2, buffer2_len);

  cff_print_console("%s", buffer2);
}