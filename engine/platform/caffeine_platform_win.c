#include "caffeine_platform.h"
#include "../core/caffeine_logging.h"

#ifdef CFF_WINDOWS

#include <Windows.h>
#include <Windowsx.h>
#include <assert.h>
#include <shlobj.h>
#include <stdarg.h>
#include <stdio.h>

#define PRINT_BUFER_LEN 3200

static LPCSTR window_class_name = "caffeine_window_class";

char app_directory[MAX_PATH] = {0};
char root_directory[MAX_PATH] = {0};

typedef struct
{
  HINSTANCE h_instance;
  HWND hwnd;
} win_platform;

LRESULT CALLBACK win32_proccess_message(HWND hwnd, UINT mesage, WPARAM w_param, LPARAM l_param);

static HWND _cff_windowns_create_window(HINSTANCE h_instance, LPCSTR window_title)
{
  HICON icon = LoadIcon(h_instance, IDI_APPLICATION);
  WNDCLASSA window_class = {0};

  window_class.style = CS_DBLCLKS;
  window_class.lpfnWndProc = win32_proccess_message;
  window_class.cbClsExtra = 0;
  window_class.cbWndExtra = 0;
  window_class.hInstance = h_instance;
  window_class.hIcon = icon;
  window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
  window_class.hbrBackground = NULL;
  window_class.lpszClassName = window_class_name;

  if (!RegisterClassA(&window_class))
  {
    caff_log(LOG_LEVEL_ERROR, "Failed to get register window class\n");
    MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
    return NULL;
  }

  caff_log(LOG_LEVEL_TRACE, "Window class registered: %s\n", window_class_name);

  int32_t client_x = 800;
  int32_t client_y = 600;
  int32_t client_width = 800;
  int32_t client_height = 600;

  int32_t window_x = client_x;
  int32_t window_y = client_y;
  int32_t window_width = client_width;
  int32_t window_height = client_height;

  uint32_t window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
  uint32_t window_ex_style = WS_EX_APPWINDOW;

  RECT border_rect = {0};
  AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

  window_x += border_rect.left;
  window_y += border_rect.top;
  window_width += border_rect.right - border_rect.left;
  window_height += border_rect.bottom - border_rect.top;

  caff_log(LOG_LEVEL_TRACE, "Window size: %d, %d, %d, %d\n", window_x, window_y, window_width, window_height);

  HWND window_hwnd = CreateWindowExA(
      window_ex_style, window_class_name, window_title,
      window_style, window_x, window_y, window_width,
      window_height, 0, 0, h_instance, 0);

  if (window_hwnd == INVALID_HANDLE_VALUE || window_hwnd == 0)
  {
    MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
    return NULL;
  }

  caff_log(LOG_LEVEL_TRACE, "Window handle: %lld\n", window_hwnd);

  int show_window = true ? SW_SHOW : SW_SHOWNOACTIVATE;

  ShowWindow(window_hwnd, show_window);

  caff_log(LOG_LEVEL_TRACE, "Window open\n");

  return window_hwnd;
}

bool cff_platform_init(platform *plat, char *name)
{
  caff_log(LOG_LEVEL_TRACE, "Init platform system\n");

  win_platform *win_plat = (win_platform *)cff_malloc((uint64_t)sizeof(win_platform));

  if (win_plat == NULL)
  {
    caff_log(LOG_LEVEL_ERROR, "Failed to allocate platform\n");
    return false;
  }

  win_plat->h_instance = GetModuleHandleA(0);

  if (win_plat->h_instance == INVALID_HANDLE_VALUE)
  {
    caff_log(LOG_LEVEL_ERROR, "Failed to get module handle\n");
    return false;
  }

  win_plat->hwnd = _cff_windowns_create_window(win_plat->h_instance, (LPCSTR)name);

  if (win_plat->hwnd == NULL)
  {
    caff_log(LOG_LEVEL_ERROR, "Failed to create window\n");
    return false;
  }

  caff_log(LOG_LEVEL_TRACE, "Window created\n");

  plat->internal_platform = (void *)win_plat;

  caff_log(LOG_LEVEL_TRACE, "Platform initialized\n");
  return true;
}

void cff_platform_shutdown(platform *plat)
{
  win_platform *win_plat = (win_platform *)plat->internal_platform;

  if (win_plat->hwnd != NULL)
  {
    DestroyWindow(win_plat->hwnd);
    win_plat->hwnd = 0;
  }

  cff_free(win_plat);
  plat->internal_platform = 0;
}

bool cff_platform_poll_events(platform *plat)
{
  MSG message;
  while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }

  return true;
}

void cff_print_console(log_level level, char *message)
{

  HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

  CONSOLE_SCREEN_BUFFER_INFO Info;
  GetConsoleScreenBufferInfo(console_handle, &Info);

  // ERROR,WARN,DEBUG,INFO,TRACE
  static WORD levels[] = {
      FOREGROUND_RED, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
      FOREGROUND_BLUE, FOREGROUND_GREEN, FOREGROUND_INTENSITY};
  SetConsoleTextAttribute(console_handle, levels[level]);
  OutputDebugStringA(message);
  uint64_t length = strlen(message);
  LPDWORD number_written = 0;
  WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length,
                number_written, 0);

  SetConsoleTextAttribute(console_handle, Info.wAttributes);
}

void cff_print_error(log_level level, char *message)
{
  HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);

  CONSOLE_SCREEN_BUFFER_INFO Info;
  GetConsoleScreenBufferInfo(console_handle, &Info);

  // ERROR,WARN,DEBUG,INFO,TRACE
  static WORD levels[] = {
      FOREGROUND_RED, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
      FOREGROUND_BLUE, FOREGROUND_GREEN, FOREGROUND_INTENSITY};
  SetConsoleTextAttribute(console_handle, levels[level]);
  OutputDebugStringA(message);
  uint64_t length = strlen(message);
  LPDWORD number_written = 0;
  WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length,
                number_written, 0);

  SetConsoleTextAttribute(console_handle, Info.wAttributes);
}

void *cff_platform_open_file(const char *path, file_attributes attributes)
{
  LPCSTR Lpath = (LPCSTR)path;
  DWORD access = 0;

  if ((attributes & FILE_READ) != 0)
    access |= GENERIC_READ;
  if ((attributes & FILE_WRITE) != 0)
    access |= GENERIC_WRITE;

  HANDLE file_handle = CreateFile(Lpath, access, 0, NULL, OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL, NULL);

  if (file_handle == INVALID_HANDLE_VALUE)
    return NULL;

  return file_handle;
}

void *cff_platform_create_file(const char *path)
{
  LPCSTR Lpath = (LPCSTR)path;

  HANDLE file_handle = CreateFile(Lpath, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                                  CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file_handle == INVALID_HANDLE_VALUE)
    return NULL;

  return (void *)file_handle;
}

cff_err_e cff_platform_file_write(void *file, void *data, uint64_t data_size)
{
  HANDLE handler = (HANDLE)file;

  if (handler == INVALID_HANDLE_VALUE || handler == NULL)
  {
    return CFF_ERR_FILE_INVALID;
  }
  DWORD bytesWriten = 0;
  if (!WriteFile(handler, (LPCVOID)data, (DWORD)data_size,
                 (LPDWORD)(&bytesWriten), NULL))
  {
    cff_print_error(LOG_LEVEL_ERROR, "Failed to write to file\n");
    CloseHandle(handler);
  }

  return CFF_ERR_NONE;
}

cff_err_e cff_platform_file_close(void *file)
{
  HANDLE handler = (HANDLE)file;

  if (handler == INVALID_HANDLE_VALUE || handler == NULL)
  {
    return CFF_ERR_FILE_INVALID;
  }

  CloseHandle(handler);
  return CFF_ERR_NONE;
}

bool cff_platform_file_exists(const char *path)
{
  LPCSTR filePath = path;

  DWORD fileAttributes = GetFileAttributes(filePath);

  if (fileAttributes == INVALID_FILE_ATTRIBUTES)
  {
    return false;
  }

  return fileAttributes & FILE_ATTRIBUTE_DIRECTORY;
}

uint64_t cff_platform_file_size(void *file)
{
  if (file == NULL || file == INVALID_HANDLE_VALUE)
    return 0;

  return (uint64_t)GetFileSize((HANDLE)(file), NULL);
}

const char *cff_get_app_directory()
{

  LPSTR buffer = (LPSTR)app_directory;

  // Get the application directory
  DWORD length = GetModuleFileName(NULL, buffer, MAX_PATH);

  if (length == 0)
  {
    return NULL;
  }

  char *lastBackslash = strrchr(app_directory, '\\');
  if (lastBackslash != NULL)
  {
    *lastBackslash = '\0';
  }

  return app_directory;
}

cff_err_e cff_platform_file_delete(const char *path)
{
  LPCSTR filepath = (LPCSTR)path;
  if (DeleteFile(filepath))
    return CFF_ERR_NONE;

  return CFF_ERR_INVALID_OPERATION;
}

const char *cff_get_app_data_directory()
{

  if (SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, root_directory) == S_OK)
  {
    return root_directory;
  }

  return NULL;
}

LRESULT CALLBACK win32_proccess_message(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
  switch (message)
  {
  case WM_ERASEBKGND:
    return 1;

  case WM_CLOSE:
    caffeine_event_fire(EVENT_QUIT, (cff_event_data){0});
    return 0;

  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;

  case WM_SIZE:
  {
    RECT r;
    GetClientRect(hwnd, &r);
    uint32_t width = r.right - r.left;
    uint32_t left = r.bottom - r.top;
    caffeine_event_fire(EVENT_RESIZE, (cff_event_data){.window_width = width, .window_height = left});
  }
  break;

  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYUP:
  {
    // bool pressed = message == WM_SYSKEYDOWN || message == WM_KEYDOWN;
  }
  break;

  case WM_MOUSEMOVE:
  {
    uint32_t x_pos = GET_X_LPARAM(l_param);
    uint32_t y_pos = GET_Y_LPARAM(l_param);
    caffeine_event_fire(EVENT_MOUSE_MOVE, (cff_event_data){.mouse_x = x_pos, .mouse_y = y_pos});
  }
  break;

  case WM_MOUSEWHEEL:
  {
    uint32_t delta = GET_WHEEL_DELTA_WPARAM(w_param);
    if (delta != 0)
    {
      delta = (delta < 0) ? -1 : 1;
    }

    caffeine_event_fire(EVENT_MOUSE_SCROLL, (cff_event_data){.mouse_scroll = delta});
  }

  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
  {
    // bool pressed = message == WM_LBUTTONDOWN || message == WM_MBUTTONDOWN || message == WM_RBUTTONDOWN;
  }
  break;
  }

  return DefWindowProcA(hwnd, message, w_param, l_param);
}
#endif