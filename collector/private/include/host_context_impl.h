/*

*/

#ifndef HOST_CONTEXT_IMPL_H_
#define HOST_CONTEXT_IMPL_H_

#include "collector.h"
#include "collector_export.h"

#include "host_context.h"
//USING_NAMESPACE_AGGREGATOR

#include "atomic_count.h"
#include "log.h"

BEGIN_NAMESPACE_COLLECTOR

//////////////////////////////////////////////////////////////////////////

USING_BASE(atomic_count);
USING_BASE(LogBaseThreadSafe);

USING_AGGREGATOR(IInterrupt);
USING_AGGREGATOR(IHostContext);
USING_AGGREGATOR(INetHandler);
//////////////////////////////////////////////////////////////////////////
class InterruptImpl : public IInterrupt {
public:
  bool_t didInterrupt() { return m_bInterrupt; }

public:
  uint32_t InterruptNO() { return m_nNo;}
  const char_t* InterruptInfo() const { return m_strMsg; }

public:
  InterruptImpl(bool_t bInterrupt = FALSE)
    : m_bInterrupt(bInterrupt)
    , m_nNo(0)
    , m_strMsg(NULL_STR)
  {}

public:
  void Interrupt(uint32_t nNo, const char_t* strMsg) {
    m_nNo = nNo;
    m_strMsg = strMsg;
    m_bInterrupt = TRUE;
  }

private:
  bool_t          m_bInterrupt;
  uint32_t        m_nNo;
  const char_t*   m_strMsg;
}; // InterruptImpl

class HostContextImpl : public IHostContext {

  // IHostContext
public:
  void Release() {}

public:
  rc_t Log(ILogWriter::log_lv_t lv, const uint8_t*, uint32_t);
  rc_t Log(ILogWriter::log_lv_t lv, const char_t* fmt, ...);
  const char_t* GetSystemExeFullPath() const;

public:
  INetHandler* GetNetHandler();

  // HostContextImpl
public:
  HostContextImpl(const char_t* strSystemPath);
  ~HostContextImpl();

  //DEFINE_REF_COUNT_RELEASE_DEL_SELF;

  // log
public:
  void ReadLog();

private:
  LogBaseThreadSafe           m_LogHost;

private:
  INetHandler*    m_NetHandler;
  char_t          m_strSystemPath[kMAX_PATH + 1];

  DISALLOW_COPY_AND_ASSIGN(HostContextImpl);
}; // HostContextImpl

END_NAMESPACE_COLLECTOR
#endif // HOST_CONTEXT_IMPL_H_
