#include "caffeine_filesystem.h"
#include "../platform/caffeine_platform.h"
#include "string.h"
#include "caffeine_logging.h"

const char dir_sep =
#ifdef CFF_WINDOWS
    '\\';
#else
    '/';
#endif

#define MAX_PATH 256

// TODO: check vector errors
path_builder cff_path_create_from_app(cff_allocator allocator)
{

  const char *app_dir = cff_get_app_directory();

  if (app_dir == NULL)
  { // failed to get program directory
    caff_log_terminal(LOG_LEVEL_ERROR, "Failed to get application directory\n");
    return (path_builder){.error_code = CFF_ERR_UNKNOW};
  }

  path_builder path = {.isFile = false, .error_code = CFF_ERR_NONE};
  path.paths = cff_vector_create(sizeof(cff_string), 4, allocator);

  if (IS_ERROR(path.paths.error_code))
  {
    caff_log_terminal(LOG_LEVEL_ERROR, "Failed to allocate buffer\n");
    return (path_builder){.error_code = path.paths.error_code};
  }

  uint64_t dir_len = strlen(app_dir);

  char *last = (char *)app_dir;
  uint64_t str_counter = 0;
  uint64_t cc = 0;

  for (char *c = (char *)app_dir; cc < dir_len + 1; c++, cc++)
  {
    if (*c == dir_sep || *c == '\0')
    {

      uint64_t size = (uint64_t)(c - last);

      cff_string str = cff_string_create(last, size + 1, allocator);

      cff_err_e push_err = cff_vector_push_back(&path.paths, (uintptr_t)(&str), allocator);

      if (IS_ERROR(push_err))
      {
        cff_string_destroy(&str, allocator);
        cff_vector_destroy(path.paths, allocator);
        caff_log_terminal(LOG_LEVEL_ERROR, "Failed to build path\n");
        return (path_builder){.error_code = push_err};
      }

      if (*(c + 1) != '\0')
      {
        last = c + 1;
      }

      str_counter += size;
    }
  }

  path.string_len = str_counter + 1;

  caff_log_terminal(LOG_LEVEL_TRACE, "Application path got\n");
  return path;
}

// TODO: check vector errors
path_builder cff_path_create_from_app_data(cff_allocator allocator)
{
  const char *app_dir = cff_get_app_data_directory();

  if (app_dir == NULL)
  { // failed to get program directory
    caff_log_terminal(LOG_LEVEL_ERROR, "Failed to get application data directory\n");
    return (path_builder){.error_code = CFF_ERR_UNKNOW};
  }

  path_builder path = {.isFile = false, .error_code = CFF_ERR_NONE};
  path.paths = cff_vector_create(sizeof(cff_string), 4, allocator);

  if (IS_ERROR(path.paths.error_code))
  {
    caff_log_terminal(LOG_LEVEL_ERROR, "Failed to allocate buffer\n");
    return (path_builder){.error_code = path.paths.error_code};
  }

  uint64_t dir_len = strlen(app_dir);

  char *last = (char *)app_dir;
  uint64_t str_counter = 0;
  uint64_t cc = 0;

  for (char *c = (char *)app_dir; cc < dir_len + 1; c++, cc++)
  {
    if (*c == dir_sep || *c == '\0')
    {

      uint64_t size = (uint64_t)(c - last);

      cff_string str = cff_string_create(last, size + 1, allocator);

      cff_err_e push_err = cff_vector_push_back(&path.paths, (uintptr_t)(&str), allocator);

      if (IS_ERROR(push_err))
      {
        cff_string_destroy(&str, allocator);
        cff_vector_destroy(path.paths, allocator);
        caff_log_terminal(LOG_LEVEL_ERROR, "Failed to build path\n");
        return (path_builder){.error_code = push_err};
      }

      if (*(c + 1) != '\0')
      {
        last = c + 1;
      }

      str_counter += size;
    }
  }

  path.string_len = str_counter + 1;

  caff_log_terminal(LOG_LEVEL_TRACE, "Application data path got\n");
  return path;
}

cff_err_e cff_path_push_directory(path_builder *path, const char *directory, cff_allocator allocator)
{

  if (path->isFile)
    return CFF_ERR_INVALID_OPERATION;

  cff_string file_str = cff_string_create_literal(directory, allocator);

  cff_err_e err =
      cff_vector_push_back(&path->paths, (uintptr_t)(&file_str), allocator);

  if (err != CFF_ERR_NONE)
  {
    cff_string_destroy(&file_str, allocator);
    return err;
  }

  path->string_len += file_str.len;

  return CFF_ERR_NONE;
}

cff_err_e cff_path_push_file(path_builder *path, const char *file, cff_allocator allocator)
{
  if (path->isFile)
    return CFF_ERR_INVALID_OPERATION;

  cff_string file_str = cff_string_create_literal(file, allocator);

  cff_err_e err =
      cff_vector_push_back(&path->paths, (uintptr_t)(&file_str), allocator);

  if (err != CFF_ERR_NONE)
  {
    cff_string_destroy(&file_str, allocator);
    return err;
  }

  path->string_len += file_str.len;
  path->isFile = true;

  return CFF_ERR_NONE;
}

cff_err_e cff_path_pop(path_builder *path, cff_allocator allocator)
{

  if (path->paths.count == 0)
    return CFF_ERR_INVALID_OPERATION;

  cff_string data;
  cff_vector_get_ref(path->paths, path->paths.count - 1, (uintptr_t *)(&data));

  cff_err_e err = cff_vector_pop_back(&path->paths, allocator);

  if (err != CFF_ERR_NONE)
    return err;

  path->string_len -= data.len;

  return CFF_ERR_NONE;
}

cff_err_e cff_path_destroy(path_builder *path, cff_allocator allocator)
{
  cff_err_e err = cff_vector_destroy(path->paths, allocator);
  if (err != CFF_ERR_NONE)
    return err;

  *path = (path_builder){0};

  return CFF_ERR_NONE;
}

cff_string cff_path_to_string(path_builder path, cff_allocator allocator)
{

  cff_string str = cff_string_join((cff_string *)(path.paths.buffer),
                                   (char)dir_sep, path.paths.count, allocator);
  return str;
}

cff_file cff_file_open(const char *filepath, file_attributes attributes)
{
  cff_file file = {.attributes = attributes, .error = CFF_ERR_NONE};

  void *handler = cff_platform_open_file(filepath, attributes);

  if (handler == NULL)
  {
    file.error = CFF_ERR_FILE_OPEN;
    return file;
  }

  file.open = true;
  file.size = cff_platform_file_size(handler);
  file.handler = handler;

  return file;
}

cff_file cff_file_create(const char *filepath)
{
  cff_file file = {.attributes = FILE_READ | FILE_WRITE, .error = CFF_ERR_NONE};

  void *handler = cff_platform_create_file(filepath);

  if (handler == NULL)
  {
    file.error = CFF_ERR_FILE_OPEN;
    return file;
  }

  file.open = true;
  file.size = cff_platform_file_size(handler);
  file.handler = handler;

  return file;
}

cff_file cff_file_open_or_create(const char *filepath,
                                 file_attributes attributes)
{

  cff_file file = cff_file_open(filepath, attributes);

  if (file.error != CFF_ERR_NONE)
  {
    file = cff_file_create(filepath);
  }

  return file;
}

bool cff_file_exists(const char *filepath)
{
  return cff_platform_file_exists(filepath);
}

cff_err_e cff_file_close(cff_file *file)
{
  cff_err_e errr = cff_platform_file_close(file->handler);

  if (errr == CFF_ERR_NONE)
    file->open = false;
  return errr;
}

cff_err_e cff_file_delete(const char *filepath)
{
  return cff_platform_file_delete(filepath);
}

cff_err_e cff_file_write_line(cff_file *file, const char *line,
                              uint64_t line_lenght)
{
  return cff_platform_file_write(file->handler, (void *)line, line_lenght);
}
cff_err_e cff_file_write_string(cff_file *file, cff_string line)
{
  return cff_platform_file_write(file->handler, (void *)line.buffer, line.len);
}
cff_err_e cff_file_write_data(cff_file *file, uintptr_t data,
                              uint64_t data_size)
{
  return cff_platform_file_write(file->handler, (void *)data, data_size);
}