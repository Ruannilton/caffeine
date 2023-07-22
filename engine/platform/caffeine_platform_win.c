#include "caffeine_platform.h"

#ifdef CFF_WINDOWS

#include <Windows.h>
#include <assert.h>
#include <malloc.h>
#include <shlobj.h>
#include <stdarg.h>
#include <stdio.h>

#define PRINT_BUFER_LEN 3200
char app_directory[MAX_PATH] = {0};
char root_directory[MAX_PATH] = {0};

static inline void internal_cff_mem_copy(const void *from, void *dest,
                                         uint64_t size) {
  register const uintptr_t *f = (const uintptr_t *)(from);
  register uintptr_t *d = (uintptr_t *)(dest);

  while (size >= sizeof(uintptr_t)) {
    *(d++) = *(f++);
    size -= sizeof(uintptr_t);
  }

  register const uint8_t *char_f = (const uint8_t *)f;
  register uint8_t *char_d = (uint8_t *)d;

  while (size) {
    *(char_d++) = *(char_f++);
    size -= sizeof(uint8_t);
  }
}

void *cff_malloc(uint64_t size) { return malloc((size_t)size); }

void *cff_stack_alloc(uint64_t size) {
#ifdef CFF_MSVC
  return _malloca((size_t)size);
#elif CFF_GCC
  return alloca(size);
#endif
}

void *cff_realloc(void *ptr, uint64_t size) {
  assert(ptr != NULL);
  return realloc(ptr, (size_t)size);
}

void cff_free(void *ptr) {
  assert(ptr != NULL);
  free(ptr);
}

void cff_stack_free(void *ptr) {
  assert(ptr != NULL);

#ifdef CFF_MSVC
  _freea(ptr);
#elif CFF_GCC

#endif
}

uint64_t cff_get_size(void *ptr) {
  assert(ptr != NULL);

#ifdef CFF_MSVC
  return (uint64_t)_msize(ptr);
#else
  return (uint64_t)(0 / 0);
#endif
}

void cff_mem_copy(const void *from, void *dest, uint64_t size) {

  if (from == dest || size == 0)
    return;

  internal_cff_mem_copy(from, dest, size);
}

void cff_mem_move(const void *from, void *dest, uint64_t size) {
  register const uint8_t *f = (const uint8_t *)from;
  register uint8_t *d = (uint8_t *)dest;

  if (f == d || size == 0)
    return;

  if (d > f && ((unsigned int)(d - f)) < size) {
    int i;
    for (i = (int)size - 1; i >= 0; i--)
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

void cff_mem_set(const void *data, void *dest, uint64_t data_size,
                 uint64_t buffer_lenght) {
  assert(buffer_lenght % data_size == 0);

  uintptr_t dest_start = (uintptr_t)dest;

  for (uint64_t i = 0; i < buffer_lenght; i += data_size) {
    void *address = (void *)(dest_start + i);
    internal_cff_mem_copy(data, address, data_size);
  }
}

void cff_mem_zero(void *dest, uint64_t data_size, uint64_t buffer_lenght) {
  uint64_t zero = 0;
  cff_mem_set(&zero, dest, data_size, buffer_lenght);
}

bool cff_mem_cmp(const void *const a, const void *const b, uint64_t size) {
  register const uintptr_t *f = (const uintptr_t *)(a);
  register const uintptr_t *d = (const uintptr_t *)(b);

  if (a == b || size == 0)
    return true;

  while (size >= sizeof(uintptr_t)) {
    if (*(d++) != *(f++))
      return false;
    size -= sizeof(uintptr_t);
  }

  register const uint8_t *char_f = (const uint8_t *)f;
  register const uint8_t *char_d = (const uint8_t *)d;

  while (size) {
    if (*(char_d++) != *(char_f++))
      return false;
    size -= sizeof(uint8_t);
  }

  return true;
}

static inline void cff_print(FILE *const file, const char *const message,
#ifdef CFF_MSVC
                             va_list arg_ptr
#else
                             __builtin_va_list arg_ptr
#endif
) {
  char buffer[PRINT_BUFER_LEN] = {0};

  vsnprintf((char *const)buffer, PRINT_BUFER_LEN, message, arg_ptr);

  fprintf(file, "%s", buffer);
}

void cff_print_console(char *message, ...) {

#ifdef CFF_MSVC
  va_list arg_ptr;
#else
  __builtin_va_list arg_ptr;
#endif
  va_start(arg_ptr, message);
  cff_print(stdout, message, arg_ptr);
  va_end(arg_ptr);
}

void cff_print_debug(char *message, ...) {
#ifdef CFF_MSVC
  va_list arg_ptr;
#else
  __builtin_va_list arg_ptr;
#endif

  const char buffer[PRINT_BUFER_LEN] = {0};

  va_start(arg_ptr, message);

  const char *const msg = message;

  vsnprintf((char *const)buffer, PRINT_BUFER_LEN, msg, arg_ptr);
  va_end(arg_ptr);

  OutputDebugString(buffer);
}

void cff_print_error(char *message, ...) {
#ifdef CFF_MSVC
  va_list arg_ptr;
#else
  __builtin_va_list arg_ptr;
#endif
  va_start(arg_ptr, message);
  cff_print(stderr, message, arg_ptr);
  va_end(arg_ptr);
}

void *cff_platform_open_file(const char *path, file_attributes attributes) {
  LPCSTR Lpath = (LPCSTR)path;
  DWORD access = 0;

  if ((attributes & FILE_READ) != 0)
    access |= GENERIC_READ;
  if ((attributes & FILE_WRITE) != 0)
    access |= GENERIC_WRITE;

  HANDLE file_handle = CreateFile(Lpath, access, 0, NULL, OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL, NULL);

  return file_handle;
}

void *cff_platform_create_file(const char *path) {
  LPCSTR Lpath = (LPCSTR)path;

  HANDLE file_handle = CreateFile(Lpath, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                                  CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
  return (void *)file_handle;
}

cff_err_e cff_platform_file_write(void *file, void *data, uint64_t data_size) {
  HANDLE handler = (HANDLE)file;

  if (handler == INVALID_HANDLE_VALUE || handler == NULL) {
    return CFF_ERR_FILE_INVALID;
  }
  DWORD bytesWriten = 0;
  if (!WriteFile(handler, (LPCVOID)data, (DWORD)data_size,
                 (LPDWORD)(&bytesWriten), NULL)) {
    cff_print_error("Failed to write to file\n");
    CloseHandle(handler);
  }

  return CFF_ERR_NONE;
}

cff_err_e cff_platform_file_close(void *file) {
  HANDLE handler = (HANDLE)file;

  if (handler == INVALID_HANDLE_VALUE || handler == NULL) {
    return CFF_ERR_FILE_INVALID;
  }

  CloseHandle(handler);
  return CFF_ERR_NONE;
}

bool cff_platform_file_exists(const char *path) {
  LPCSTR filePath = path;

  DWORD fileAttributes = GetFileAttributes(filePath);

  if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
    return false;
  }

  return fileAttributes & FILE_ATTRIBUTE_DIRECTORY;
}

uint64_t cff_platform_file_size(void *file) {
  if (file == NULL || file == INVALID_HANDLE_VALUE)
    return 0;

  return (uint64_t)GetFileSize((HANDLE)(file), NULL);
}

const char *cff_get_app_directory() {

  LPSTR buffer = (LPSTR)app_directory;

  // Get the application directory
  DWORD length = GetModuleFileName(NULL, buffer, MAX_PATH);

  if (length == 0) {
    return NULL;
  }

  char *lastBackslash = strrchr(app_directory, '\\');
  if (lastBackslash != NULL) {
    *lastBackslash = '\0';
  }

  return app_directory;
}

cff_err_e cff_platform_file_delete(const char *path) {
  LPCSTR filepath = (LPCSTR)path;
  if (DeleteFile(filepath))
    return CFF_ERR_NONE;

  return CFF_ERR_INVALID_OPERATION;
}

const char *cff_get_app_data_directory() {

  if (SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, root_directory) == S_OK) {
    return root_directory;
  }

  return NULL;
}
#endif