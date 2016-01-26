/*

*/
#include "Lock.h"

BEGIN_NAMESPACE_BASE

#if defined(PLATFORM_API_WINDOWS)
# include <windows.h>

//////////////////////////////////////////////////////////////////////////
//
Lock::Lock() {

  LPCRITICAL_SECTION lpcs = (LPCRITICAL_SECTION)malloc(sizeof(CRITICAL_SECTION));
  if (NULL == lpcs) { return; }

  InitializeCriticalSection(lpcs);
  m_handle = lpcs;
}

Lock::~Lock() {

  LPCRITICAL_SECTION lpcs = (LPCRITICAL_SECTION)(m_handle);
  if (lpcs) {

    DeleteCriticalSection(lpcs);
    free(lpcs);
  }
}

void Lock::Acquire() {

  ASSERT(m_handle);
  LPCRITICAL_SECTION lpcs = (LPCRITICAL_SECTION)(m_handle);
  EnterCriticalSection(lpcs);
}

void Lock::Release() {

  ASSERT(m_handle);
  LPCRITICAL_SECTION lpcs = (LPCRITICAL_SECTION)(m_handle);
  LeaveCriticalSection(lpcs);
}

bool_t Lock::Try() {
  ASSERT(m_handle);
#if(_WIN32_WINNT >= 0x0400)
  LPCRITICAL_SECTION lpcs = (LPCRITICAL_SECTION)(m_handle);
  return TryEnterCriticalSection(lpcs);
#else
  return FALSE;
#endif
}

#else
# error "Lock"
#endif // PLATFORM_API_WINDOWS





END_NAMESPACE_BASE
