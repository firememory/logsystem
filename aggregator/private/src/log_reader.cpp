/*

*/

#include "log_reader.h"

BEGIN_NAMESPACE_AGGREGATOR

//////////////////////////////////////////////////////////////////////////
void LogReader::Notify() {

  m_eventThd.Signal();
}

void LogReader::ThreadMain(void* context) {

  ILogWriter* pILogWriter = (ILogWriter*)(context);
  if (NULL == pILogWriter) { return; }

  while (FALSE == m_bStop) {

    do 
    {

      AutoRelease<ILogWriter::ILogNode*> autoRelLogNode(pILogWriter->Get());
      if (NULL == autoRelLogNode) { break; }

      // proc log

    } while (FALSE == m_bStop);

    m_eventThd.Wait();
  }

  // exit
  m_eventThd.Signal();
}

//////////////////////////////////////////////////////////////////////////
const char_t*  LogReader::m_strThdName  = _STR("LogReader");
//////////////////////////////////////////////////////////////////////////
LogReader::LogReader()
  : Thread(m_strThdName)
  , m_eventThd(FALSE, FALSE)
  , m_bStop(FALSE)
{}

LogReader::~LogReader() {

  Stop();
}

//////////////////////////////////////////////////////////////////////////
rc_t LogReader::Start(ILogWriter* pILogWriter) {

  if (NULL == pILogWriter) { return RC_S_NULL_VALUE; }

  m_bStop = FALSE;
  return TRUE == Thread::Start(pILogWriter) ? RC_S_OK : RC_S_FAILED;
}


rc_t LogReader::Stop() {

  if (TRUE == m_bStop) { return RC_S_STATUS; }

  m_bStop = TRUE;
  m_eventThd.Signal();

  Thread::Stop();
  return RC_S_OK;
}


END_NAMESPACE_AGGREGATOR
