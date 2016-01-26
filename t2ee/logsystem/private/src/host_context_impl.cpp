/*

*/
#include "stdafx.h"
#include "host_context_impl.h"

#include "T2EEHandler.h"

BEGIN_NAMESPACE_LOGSYSTEM

//////////////////////////////////////////////////////////////////////////
// NetHandler
NetHandlerImpl::NetHandlerImpl(CT2EEHandler* pT2EEHandler, IServer* pIServer,
                               ISession* pISession, IConnect* pIConnect, IBusiness* pIBusiness, RAWANSCNTX* pRawAnsCntx)
  : m_pT2EEHandler(pT2EEHandler)
  , m_pIServer(pIServer)

  , m_pISession(pISession)
  , m_pIConnect(pIConnect)
  , m_pIBusiness(pIBusiness)
  , m_pRawAnsCntx(pRawAnsCntx)
{}

NetHandlerImpl::~NetHandlerImpl() { }

void NetHandlerImpl::Release() {}

uint32_t NetHandlerImpl::GetSessionID() const {

  ASSERT(m_pIConnect);
  return m_pIConnect->GetConnectID();
}

const uint8_t* NetHandlerImpl::GetLocalIP(size_t* len) {

  if (NULL == m_pIConnect) { return NULL; }
  ISession* pISession = m_pIConnect->QueryRelatedSession();

  if (NULL == pISession) { return NULL; }
  IUser* pIUser = pISession->GetCreator();
  if (NULL == pIUser) { return NULL; }

  IP localIP = pIUser->GetEthernetIp();
  const uint8_t* pIP = (const uint8_t*)(&localIP);
  m_ipLocal.reserve(sizeof(IP));
  m_ipLocal.assign(pIP, pIP + sizeof(IP));

  if (len) { (*len) = m_ipLocal.size(); }
  return &m_ipLocal.front();
}

const uint8_t* NetHandlerImpl::GetLocalMAC(size_t* len) {

  if (NULL == m_pIConnect) { return NULL; }
  ISession* pISession = m_pIConnect->QueryRelatedSession();

  if (NULL == pISession) { return NULL; }
  IUser* pIUser = pISession->GetCreator();
  if (NULL == pIUser) { return NULL; }

  MAC localMAC = pIUser->GetMac();
  const uint8_t* pMAC = (const uint8_t*)(&localMAC);
  m_macLocal.reserve(sizeof(MAC));
  m_macLocal.assign(pMAC, pMAC + sizeof(MAC));

  if (len) { (*len) = m_macLocal.size(); }
  return &m_macLocal.front();
}

const uint8_t* NetHandlerImpl::GetTargetIP(size_t* len) {

  if (NULL == m_pISession) { return NULL; }
  IUser* pIUser = m_pISession->GetCreator();
  if (NULL == pIUser) { return NULL; }

  IP localIP = pIUser->GetInternetIp();
  const uint8_t* pIP = (const uint8_t*)(&localIP);
  m_ipTarget.reserve(sizeof(IP));
  m_ipTarget.assign(pIP, pIP + sizeof(IP));

  if (len) { (*len) = m_ipTarget.size(); }
  return &m_ipTarget.front();
}

const uint8_t* NetHandlerImpl::GetTargetMAC(size_t* len) {

  if (NULL == m_pISession) { return NULL; }
  IUser* pIUser = m_pISession->GetCreator();
  if (NULL == pIUser) { return NULL; }

  MAC localMAC = pIUser->GetMac();
  const uint8_t* pMAC = (const uint8_t*)(&localMAC);
  m_macTarget.reserve(sizeof(MAC));
  m_macTarget.assign(pMAC, pMAC + sizeof(MAC));

  if (len) { (*len) = m_macTarget.size(); }
  return &m_macTarget.front();
}

rc_t NetHandlerImpl::Send(const void* context, const uint8_t* data, uint32_t len) {

  UNUSED_PARAM(context);
  ASSERT(data && len);
  ASSERT(m_pRawAnsCntx);
  if(len > m_pRawAnsCntx->m_cbBodyMax) {

    ASSERT(m_pIServer);
    BPR error = m_pIServer->ThrowProtocolError(m_pIConnect,m_pIBusiness,SYS_ERR_BUFFERTOOSMALL,"»º³åÇøÌ«Ð¡¡£");
    RC_RETURN(RC_S_FAILED + error);
  }

  m_pRawAnsCntx->m_cbBody = len;
  memcpy(m_pRawAnsCntx->m_pBody, data, len);

  ASSERT(m_pT2EEHandler);
  m_pT2EEHandler->SetBusinessCompleted(m_pIBusiness, m_pRawAnsCntx, len);
  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
// HostContextImpl
void HostContextImpl::Release() {}

rc_t HostContextImpl::Log(ILogWriter::log_lv_t lv, const uint8_t* pLogData, uint32_t len) {

  if (NULL == m_pIServer) { return RC_S_NULL_VALUE; }

  UNUSED_PARAM(lv);
  m_pIServer->AddRawCallLog(TRUE, (LPBYTE)pLogData, len);
  return RC_S_OK;
}

rc_t HostContextImpl::Log(ILogWriter::log_lv_t lv, const char_t* fmt, ...) {

  if (NULL == m_pIServer) { return RC_S_NULL_VALUE; }

  va_list arglist;
  va_start(arglist, fmt);
  if (lv == ILogWriter::LV_TRACE) { m_pIServer->DirectTracerV(fmt, arglist); }
  else if (lv > ILogWriter::LV_WARN) { m_pIServer->AddInfoLogV(fmt, arglist); }
  else { m_pIServer->AddErrorLogV(fmt, arglist); }
  va_end(arglist);

  return RC_S_OK;
}

const char_t* HostContextImpl::GetSystemExeFullPath() const {

#if defined(DEBUG_ON)
  return "../Debug";
#else

  if (NULL == m_pIServer) { return NULL_STR; }

  const char_t* strHomeDir = m_pIServer->GetHomeDir();
  return NULL == strHomeDir ? NULL_STR : strHomeDir;
#endif // DEBUG_ON
}


HostContextImpl::HostContextImpl(CT2EEHandler* pT2EEHandler, IServer* pIServer)
  : m_pT2EEHandler(pT2EEHandler)
  , m_pIServer(pIServer)
{}

HostContextImpl::~HostContextImpl() { }

END_NAMESPACE_LOGSYSTEM
