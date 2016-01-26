/*

*/

#ifndef WAITABLE_EVENT_H_
#define WAITABLE_EVENT_H_

#include "base.h"
#include "base_export.h"

BEGIN_NAMESPACE_BASE

class BASE_EXPORT WaitableEvent {
public:
  // If manual_reset is true, then to set the event state to non-signaled, a
  // consumer must call the Reset method.  If this parameter is false, then the
  // system automatically resets the event state to non-signaled after a single
  // waiting thread has been released.
  WaitableEvent(bool_t manual_reset, bool_t initially_signaled);

  ~WaitableEvent();

public:
  // Put the event in the un-signaled state.
  void Reset();

  // Put the event in the signaled state.  Causing any thread blocked on Wait
  // to be woken up.
  void Signal();

  // Returns true if the event is in the signaled state, else false.  If this
  // is not a manual reset event, then this test will cause a reset.
  bool_t IsSignaled();

  // Wait indefinitely for the event to be signaled.
  void Wait();

  // Wait up until max_time has passed for the event to be signaled.  Returns
  // true if the event was signaled.  If this method returns false, then it
  // does not necessarily mean that max_time was exceeded.
  bool_t TimedWait(uint32_t max_time_millisecond);

private:

  typedef void*     handle_t;
  handle_t    handle_;

  DISALLOW_COPY_AND_ASSIGN(WaitableEvent);
}; // WaitableEvent

END_NAMESPACE_BASE

#endif // WAITABLE_EVENT_H_
