/*
  dll so
*/
#ifndef DYNAMIC_LIBRARY_H_
#define DYNAMIC_LIBRARY_H_

#include "base.h"
//#include "base_export.h"

BEGIN_NAMESPACE_BASE

BASE_EXPORT rc_t AppendSystemPath(const char_t*);
BASE_EXPORT rc_t RemoveSystemPath(const char_t*);

#if defined(DEBUG_ON)
BASE_EXPORT void trace_system_error();
#define TRACE_SYSTEM_ERROR  trace_system_error()
#else
#define TRACE_SYSTEM_ERROR
#endif

class BASE_EXPORT DynamicLibrary {
public:
  DynamicLibrary(const char_t* file_name = NULL, const char_t* full_path = NULL);
  ~DynamicLibrary();

public:
  bool_t isValid() const { return dl_handle_ ? TRUE : FALSE; }
  rc_t open(const char_t* file_name, const char_t* full_path = NULL);
  void close(); 

public:
  /*
  template<typename TargType>
  inline TargType LocateSymbol(const char_t* symbol) {
    return reinterpret_cast<TargType>( locateSymbol_(symbol) );
  }
  */
  inline void* LocateSymbol(const char_t* symbol) {
    return locateSymbol_(symbol);
  }

  const char_t* LibraryName() const { return strLibraryName_; }

private:
  typedef void*     handle_t;
  handle_t locateSymbol_(const char_t* symbol);

  handle_t  dl_handle_;
  bool_t    bSetPath_;
  char_t    strCurrFullPath_[kMAX_PATH];
  char_t    strLibraryName_[kMAX_PATH];

  DISALLOW_COPY_AND_ASSIGN(DynamicLibrary);
}; // DynamicLibrary

END_NAMESPACE_BASE

#endif // DYNAMIC_LIBRARY_H_
