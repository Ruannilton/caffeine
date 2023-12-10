#pragma once

#if _WIN32 || _WIN64 || __CYGWIN__
#define CFF_WINDOWS
#endif

#if unix || __unix || __unix__
#define CFF_UNIX
#endif

#if __linux__ || linux || __linux
#define CFF_LINUX
#endif

#if __clang__
#define CFF_CLANG
#endif

#if __GNUC__
#define CFF_GCC
#endif

#if _MSC_VER
#define CFF_MSVC
#endif

#ifdef __cplusplus
#define CFF_CPP
#else
#endif
#define CFF_C

#ifdef CFF_CLANG
#define __CFF_FILE_NAME__ __FILE_NAME__
#else
#define __CFF_FILE_NAME__ __FILE__
#endif
