/*

*/
#include "waitable_event.h"

BEGIN_NAMESPACE_BASE

#if defined(PLATFORM_API_WINDOWS)
# include <windows.h>

//////////////////////////////////////////////////////////////////////////
//
WaitableEvent::WaitableEvent(bool_t manual_reset, bool_t signaled)
  : handle_(CreateEvent(NULL, manual_reset, signaled, NULL)) {

  ASSERT(handle_);
}

WaitableEvent::~WaitableEvent() {
  CloseHandle(handle_);
}

void WaitableEvent::Reset() {
  ResetEvent(handle_);
}

void WaitableEvent::Signal() {
  SetEvent(handle_);  
}

bool_t WaitableEvent::IsSignaled() {
  return TimedWait(0);
}

void WaitableEvent::Wait() {

  /*DWORD result = */WaitForSingleObject(handle_, INFINITE);
}

bool_t WaitableEvent::TimedWait(uint32_t max_time_millisecond) {

  DWORD result = WaitForSingleObject(handle_, max_time_millisecond);
  switch (result) {
    case WAIT_OBJECT_0:
      return TRUE;
    case WAIT_TIMEOUT:
      return FALSE;
  }
  return FALSE;
}

#else
# error "WaitableEvent"
#endif // PLATFORM_API_WINDOWS


END_NAMESPACE_BASE
