/*

*/
#include "Thread.h"

#include "waitable_event.h"

BEGIN_NAMESPACE_BASE

#if defined(PLATFORM_API_WINDOWS)
# define __WIN__
# include <windows.h>

//////////////////////////////////////////////////////////////////////////
//
typedef uint32_t    PlatformThreadId;

// The information on how to set the thread name comes from
// a MSDN article: http://msdn2.microsoft.com/en-us/library/xcb2z8hs.aspx
const DWORD kVCThreadNameException = 0x406D1388;

typedef struct tagTHREADNAME_INFO {
  DWORD dwType;  // Must be 0x1000.
  LPCSTR szName;  // Pointer to name (in user addr space).
  DWORD dwThreadID;  // Thread ID (-1=caller thread).
  DWORD dwFlags;  // Reserved for future use, must be zero.
} THREADNAME_INFO;

// This function has try handling, so it is separated out of its caller.
void SetNameInternal(PlatformThreadId thread_id, const char* name) {
  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = name;
  info.dwThreadID = thread_id;
  info.dwFlags = 0;

  __try {
    RaiseException(kVCThreadNameException, 0, sizeof(info)/sizeof(DWORD),
      reinterpret_cast<DWORD_PTR*>(&info));
  } __except(EXCEPTION_CONTINUE_EXECUTION) {
  }
}

struct ThreadParams {
  Thread* delegate;
  Thread* delegate_thdmain;
  void*   context;
  WaitableEvent event;

  ThreadParams(Thread* delegate_, Thread* delegate__, void* context_)
    : delegate(delegate_)
    , delegate_thdmain(delegate__)
    , context(context_)
    , event(FALSE, FALSE)
  {}
};

DWORD __stdcall ThreadFunc(void* params) {
  ThreadParams* thread_params = static_cast<ThreadParams*>(params);
  ASSERT(thread_params);
  if (NULL == thread_params) { return NULL;}
  Thread* delegate = thread_params->delegate_thdmain;
  Thread* delegate_ins = thread_params->delegate;
  void* context = thread_params->context;
  if (FALSE == delegate_ins->Init()) { return NULL; }
  thread_params->event.Signal();
//   PlatformThread::Delegate* delegate = thread_params->delegate;
//   if (!thread_params->joinable)
//     base::ThreadRestrictions::SetSingletonAllowed(false);
//   delete thread_params;
  delegate->ThreadMain(context);
  delegate_ins->DeInit();
  return NULL;
}

namespace {

  typedef void*                         handle_t;
  typedef handle_t                      PlatformThreadHandle;

  bool_t CreateThreadInternal(size_t stack_size,
    ThreadParams* delegate,
    PlatformThreadHandle* out_thread_handle) {
      PlatformThreadHandle thread_handle;
      unsigned int flags = 0;
      if (stack_size > 0/* && base::win::GetVersion() >= base::win::VERSION_XP*/) {
        flags = STACK_SIZE_PARAM_IS_A_RESERVATION;
      } else {
        stack_size = 0;
      }

//       ThreadParams* params = new ThreadParams;
//       params->delegate = delegate;
//       params->joinable = out_thread_handle != NULL;

      ThreadParams* params = delegate;

      // Using CreateThread here vs _beginthreadex makes thread creation a bit
      // faster and doesn't require the loader lock to be available.  Our code will
      // have to work running on CreateThread() threads anyway, since we run code
      // on the Windows thread pool, etc.  For some background on the difference:
      //   http://www.microsoft.com/msj/1099/win32/win321099.aspx
      thread_handle = CreateThread(
        NULL, stack_size, ThreadFunc, params, flags, NULL);
      if (!thread_handle) {
        /*delete params;*/
        return FALSE;
      }

      if (out_thread_handle)
        *out_thread_handle = thread_handle;
      else
        CloseHandle(thread_handle);
      return TRUE;
  }

  void JoinThreadInternal(PlatformThreadHandle thread_handle) {
    // Wait for the thread to exit.  It should already have terminated but make
    // sure this assumption is valid.
    DWORD result = WaitForSingleObject(thread_handle, INFINITE);
    if (result != WAIT_OBJECT_0) {
      // Debug info for bug 127931.
//       DWORD error = GetLastError();
//       debug::Alias(&error);
//       debug::Alias(&result);
//       debug::Alias(&thread_handle);
//       CHECK(false);
    }

    CloseHandle(thread_handle);
  }
}

bool_t Thread::Start(Thread* thd, void* context) {

  ThreadParams thread_params(this, thd, context);

  if (FALSE == CreateThreadInternal(0, &thread_params, &m_handle)) {
    return FALSE;
  }

  thread_params.event.Wait();

  m_bRunning = m_bStarted;
  return m_bStarted;
}

bool_t Thread::Start(void* context) {

  ThreadParams thread_params(this, this, context);

  if (FALSE == CreateThreadInternal(0, &thread_params, &m_handle)) {
    return FALSE;
  }

  thread_params.event.Wait();

  return m_bStarted;
}

void Thread::Stop() {

  if (!m_bStarted)
    return;

  JoinThreadInternal(m_handle);
  m_handle = NULL;
  m_bStarted = FALSE;  
}

bool_t Thread::Init() {
  
  m_bStarted = TRUE;

  SetNameInternal(GetCurrentThreadId(), Name());
  return TRUE;
}

void Thread::DeInit() {

  m_bRunning = FALSE;
}

#else
# include <syswait.h>
#endif // PLATFORM_API_WINDOWS

//////////////////////////////////////////////////////////////////////////

Thread::Thread(const char_t* name)
  : m_bStarted(FALSE)
  , m_bRunning(FALSE)
  , m_handle(NULL)
{
  if (name) {
    STRCPY(name_, MAX_NAME_LEN, name);
  }
  else {
    STRCPY(name_, MAX_NAME_LEN, NULL_STR);
  }
  name_[MAX_NAME_LEN] = END_CHAR;
}

Thread::~Thread() {
  Stop();
}

void Thread::sleep(uint32_t millisecond) {

#if defined(PLATFORM_API_WINDOWS)
  Sleep(millisecond);
#else // linux
  sleep(millisecond);
#endif 
}

END_NAMESPACE_BASE
