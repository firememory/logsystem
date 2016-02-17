/*

*/

#ifndef LOG_READER_H_
#define LOG_READER_H_

#include "aggregator_base.h"

#include "object.h"
#include "thread.h"
#include "waitable_event.h"

//////////////////////////////////////////////////////////////////////////
BEGIN_NAMESPACE_AGGREGATOR

USING_BASE(WaitableEvent);
USING_BASE(Thread);


//////////////////////////////////////////////////////////////////////////
class LogReader : public Thread, public IListener {

  // ILogListener
public:
  void Release() {}

public:
  void Notify();

  // Thread
public:
  void ThreadMain(void*);

public:
  LogReader();
  ~LogReader();

public:
  rc_t Start(ILogWriter*);
  rc_t Stop();

private:
  WaitableEvent         m_eventThd;

  volatile  bool_t      m_bStop;
  static const char_t*  m_strThdName;
}; // LogReader

END_NAMESPACE_AGGREGATOR

#endif // LOG_READER_H_
