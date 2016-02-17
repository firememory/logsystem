/*

*/

#ifndef WINDOWS_DEF_H_
#define WINDOWS_DEF_H_

#include <config.h>

#if !defined(USE_WINDOWS_DEF)

typedef char_t*                         LPSTR;
typedef const char_t*                   LPCSTR;

typedef int32_t                         LONG;
typedef uint32_t                        DWORD;
typedef uint32_t                        UINT;
typedef bool_t                          BOOL;

typedef DWORD*                          LPDWORD;
typedef void*                           LPVOID;

#ifndef WINAPI
#define WINAPI                          __stdcall
#endif // WINAPI

#ifndef MAX_PATH
#define MAX_PATH  260
#endif

#endif // USE_WINDOWS_DEF

#endif // WINDOWS_DEF_H_
