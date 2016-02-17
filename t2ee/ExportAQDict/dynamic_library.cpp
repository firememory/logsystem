/*

*/
#include "StdAfx.h"
#include "dynamic_library.h"

BEGIN_NAMESPACE_BASE

#if defined(PLATFORM_API_WINDOWS) // WINDOWS API
# include <Windows.h>
#elif defined(PLATFORM_API_POSIX)  // POSIX API
# include <dlfcn.h>
#endif // PLATFORM_API

DynamicLibrary::DynamicLibrary(const char_t* file_name /* = NULL */, const char_t* full_path /* = NULL */)
  : dl_handle_(NULL)
  , bSetPath_(FALSE)
{
  BZERO_ARR(strCurrFullPath_);
  BZERO_ARR(strLibraryName_);
  open(file_name, full_path);
}

DynamicLibrary::~DynamicLibrary() { close(); }

rc_t DynamicLibrary::open(const char_t* file_name, const char_t* full_path) {

  if (NULL == file_name || dl_handle_) { return RC_S_NULL_VALUE; }

  STRCPY(strLibraryName_, sizeof(strLibraryName_), file_name);

  if (full_path && STRLEN(full_path) < sizeof(strCurrFullPath_)) {

    if (TRUE == bSetPath_) { RemoveSystemPath(strCurrFullPath_); }
    bSetPath_ = RC_S_OK == AppendSystemPath(full_path) ? TRUE : FALSE;
    if (TRUE == bSetPath_) { STRCPY(strCurrFullPath_, sizeof(strCurrFullPath_), full_path); }
  }

#if defined(PLATFORM_API_WINDOWS) // WINDOWS API
  dl_handle_ = LoadLibrary(file_name);
  TRACE_SYSTEM_ERROR;
#elif defined(PLATFORM_API_POSIX)  // POSIX API
  dl_handle_ = dlopen(dl_handle_, RTLD_LOCAL | RTLD_LAZY);
  TRACE(dlerror());
# else
#   error "Dynamic library API is unknown"
#endif // PLATFORM_API

  return dl_handle_ ? RC_S_OK : RC_S_FAILED;
}

void DynamicLibrary::close() {

  if (NULL == dl_handle_) { return;}

#if defined(PLATFORM_API_WINDOWS) // WINDOWS API
  FreeLibrary((HINSTANCE)dl_handle_);
#elif defined(PLATFORM_API_POSIX)  // POSIX API
  dlclose(dl_handle_);
# else
#    error "Dynamic library API is unknown"
#endif // PLATFORM_API

  if (TRUE == bSetPath_) { RemoveSystemPath(strCurrFullPath_); }
}

DynamicLibrary::handle_t DynamicLibrary::locateSymbol_(const char_t* symbol) {

  if (NULL == symbol || NULL == dl_handle_) { return NULL;}

#if defined(PLATFORM_API_WINDOWS) // WINDOWS API
  return GetProcAddress((HINSTANCE)dl_handle_, symbol);
#elif defined(PLATFORM_API_POSIX)  // POSIX API
  return dlsym((HINSTANCE)dl_handle_, symbol);
# else
#   error "Dynamic library API is unknown"
#endif // PLATFORM_API

}


static const char_t pathVariable[] = _STR("PATH");
static const char_t pathVariableSep[] = _STR(";");
static const uint32_t pathVariableSepLen = sizeof(pathVariableSep) - sizeof(char_t);

rc_t AppendSystemPath(const char_t* path) {

#if defined(PLATFORM_API_WINDOWS) // WINDOWS API
  uint32_t length = GetEnvironmentVariable(pathVariable, 0, 0);
  if (!length) {
    return SetEnvironmentVariable(pathVariable, path) ? RC_S_OK : RC_S_FAILED;
  }
  
  uint32_t nNewPathLen = length + pathVariableSepLen + STRLEN(path) + pathVariableSepLen + sizeof(char_t);
  //char_t* strNewPath = (char_t*)malloc(nNewPathLen);
  char_t strNewPath[4*1024] = {0};
  if (nNewPathLen >= 4*1024) { return RC_E_NOMEM; }
  if (NULL == strNewPath || 
    !GetEnvironmentVariable(pathVariable, strNewPath, nNewPathLen)
  ) {
    return RC_S_FAILED;
  }

  const char_t* strFind = STRSTR(strNewPath, sizeof(strNewPath), path);
  if (strFind) { return RC_S_DUPLICATE; }

  STRCAT(strNewPath, nNewPathLen, pathVariableSep);
  STRCAT(strNewPath, nNewPathLen, path);
  STRCAT(strNewPath, nNewPathLen, pathVariableSep);

  rc_t rc = SetEnvironmentVariable(pathVariable, strNewPath) ? RC_S_OK : RC_S_FAILED;
  //free(strNewPath);
  return rc;
#else
# error "AppendSystemPath is unknown!"
#endif
}

rc_t RemoveSystemPath(const char_t* path) {

#if defined(PLATFORM_API_WINDOWS) // WINDOWS API
  char_t strPath[kMAX_PATH + 1] = {0};

  uint32_t length = GetEnvironmentVariable(pathVariable, 0, 0);
  if (!length || strlen(path) > MAX_PATH) {
    return RC_E_NOMEM;
  }

  uint32_t nNewPathLen = length + sizeof(char_t);
  char_t* strNewPath = (char_t*)malloc(nNewPathLen);
  if (NULL == strNewPath || 
    !GetEnvironmentVariable(pathVariable, strNewPath, nNewPathLen)
  ) {

    free(strNewPath); return RC_S_FAILED;
  }

  STRCAT(strPath, MAX_PATH, path);
  STRCAT(strPath, MAX_PATH, pathVariableSep);  

  char_t* strFind = STRSTR(strNewPath, sizeof(strNewPath), strPath);
  if (NULL == strFind) {
    free(strNewPath);  return RC_S_FAILED;
  }

  nNewPathLen = (uint32_t)STRNLEN(strPath, sizeof(strPath));
  MEMMOVE(strFind, length - (strFind - strNewPath)
    , strFind + nNewPathLen
    , length - (strFind - strNewPath) - nNewPathLen
    );

  rc_t rc = SetEnvironmentVariable(pathVariable, strNewPath) ? RC_S_OK : RC_S_FAILED;
  free(strNewPath);
  return rc;
#else
# error "RemoveSystemPath is unknown!"
#endif
}

void trace_system_error() {

#if defined(PLATFORM_API_WINDOWS) // WINDOWS API
  LPVOID lpMsgBuf;
  FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0,
    NULL 
    );
  // Process any inserts in lpMsgBuf.
  // ...
  // Display the string.
  //MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
  TRACE((LPCTSTR)lpMsgBuf);
  // Free the buffer.
  LocalFree( lpMsgBuf );
#else
# error "system error is unknown!"
#endif
}

END_NAMESPACE_BASE
