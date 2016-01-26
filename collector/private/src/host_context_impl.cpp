/*

*/

#include "host_context_impl.h"

#include "time_util.h"

BEGIN_NAMESPACE_COLLECTOR

USING_BASE(micro_time);

//////////////////////////////////////////////////////////////////////////
rc_t HostContextImpl::Log(ILogWriter::log_lv_t lv, const uint8_t* pLogData, uint32_t nLogLen) {

  return m_LogHost.Log(lv, pLogData, nLogLen);
}

rc_t HostContextImpl::Log(ILogWriter::log_lv_t lv, const char_t* fmt, ...) {

  rc_t rc;
  va_list arglist;
  va_start(arglist, fmt);
  rc = m_LogHost.Log(lv, fmt, arglist);
  va_end(arglist);

  return rc;
}

const char_t* HostContextImpl::GetSystemExeFullPath() const { return m_strSystemPath; }

INetHandler* HostContextImpl::GetNetHandler() { return m_NetHandler; }

//////////////////////////////////////////////////////////////////////////
HostContextImpl::HostContextImpl(const char_t* strSystemPath)
  : m_NetHandler(NULL)
  //, REF_COUNT_CONSTRUCT
  , m_LogHost()
{
  BZERO_ARR(m_strSystemPath);

  if (strSystemPath) {
    STRCPY(m_strSystemPath, kMAX_PATH, strSystemPath);
  }
}


HostContextImpl::~HostContextImpl() {}

//////////////////////////////////////////////////////////////////////////
void HostContextImpl::ReadLog() {

  ILogWriter::ILogNode* pILogNode = NULL;
  while (NULL != (pILogNode = m_LogHost.Get())) {

    char_t strTime[micro_time::TIME_FORAT_LEN];
    micro_time::format(strTime, pILogNode->Time64());

    const IMemoryNode* pIMemoryNode = pILogNode->MemNode();
    if (NULL == pIMemoryNode) { continue; }

    PRINTF("%s [%s], %s\n", strTime, kStrLOG_LV[pILogNode->Type()], pIMemoryNode->data());
    pILogNode->Release();
  }

}

END_NAMESPACE_COLLECTOR
