#pragma once

#include "../caffeine_types.h"
#include "../collections/caffeine_vector.h"
#include "caffeine_string.h"

typedef struct {
  cff_vector_s paths;
  bool isFile;
  uint64_t string_len;
} path_builder;

typedef struct {
  uint64_t size;
  file_attributes attributes;
  bool open;
  cff_err_e error;
  void *handler;
} cff_file;

path_builder cff_path_create_from_app(cff_allocator_t allocator);

path_builder cff_path_create_from_app_data(cff_allocator_t allocator);

cff_err_e cff_path_push_directory(path_builder *path, const char *directory,
                                  cff_allocator_t allocator);

cff_err_e cff_path_push_file(path_builder *path, const char *file,
                             cff_allocator_t allocator);

cff_err_e cff_path_pop(path_builder *path, cff_allocator_t allocator);

cff_err_e cff_path_destroy(path_builder *path, cff_allocator_t allocator);

cff_string cff_path_to_string(path_builder path, cff_allocator_t allocator);

cff_file cff_file_open(const char *filepath, file_attributes attributes);

cff_file cff_file_create(const char *filepath);

cff_file cff_file_open_or_create(const char *filepath,
                                 file_attributes attributes);

bool cff_file_exists(const char *filepath);

cff_err_e cff_file_close(cff_file *file);

cff_err_e cff_file_delete(const char *filepath);

cff_err_e cff_file_write_line(cff_file *file, const char *line,
                              uint64_t line_lenght);

cff_err_e cff_file_write_string(cff_file *file, cff_string line);

cff_err_e cff_file_write_data(cff_file *file, uintptr_t data,
                              uint64_t data_size);
