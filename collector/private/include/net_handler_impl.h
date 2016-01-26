/*

*/

#ifndef NET_HANDLER_IMPL_H_
#define NET_HANDLER_IMPL_H_

#include "collector.h"
#include "collector_export.h"

#include "net_handler.h"
#include "host_context.h"

#ifndef WINAPI
#define WINAPI                          __stdcall
#endif // WINAPI

#include "atomic_count.h"
#include "dynamic_library.h"
#include "time_util.h"

// net
#include "net_util.h"

namespace tdx { 
  namespace taapi {
    class IEngine;
    class IClient;
  } // taapi
} // tdx

using namespace tdx::taapi;
typedef IEngine* (*PFN_TAAPI_CREATEINSTANCE)(const char_t*, uint32_t);

BEGIN_NAMESPACE_COLLECTOR

USING_BASE(atomic_count);
USING_BASE(DynamicLibrary);
USING_BASE(Time);

USING_AGGREGATOR(INetHandler);
USING_AGGREGATOR(INetEvent);
USING_AGGREGATOR(IHostContext);

USING_NET(NetworkInterfaceList);
USING_NET(IPAddressNumber);
USING_NET(MACAddressNumber);

class NetHandlerImpl : public INetHandler {

  // INetHandler
public:
  void Release();

public:
  rc_t InitNet();
  rc_t DeInitNet();
  rc_t StartNet();
  rc_t StopNet();

public:
  rc_t Open();
  rc_t Close();

public:
  uint32_t GetSessionID() const;
  const uint8_t* GetLocalIP(size_t* len);
  const uint8_t* GetLocalMAC(size_t* len);
  const uint8_t* GetTargetIP(size_t* len);
  const uint8_t* GetTargetMAC(size_t* len);

public:
  rc_t Send(const void* context, const uint8_t* data, uint32_t len);
  rc_t SendTo(const void* /*context*/, uint32_t /*id*/, const uint8_t* /*data*/, uint32_t /*len*/) { return RC_E_UNSUPPORTED; }

  rc_t Recv(uint8_t* /*data*/, uint32_t* /*len*/) { return RC_E_UNSUPPORTED; }
  rc_t RecvForm(uint32_t /*id*/, uint8_t* /*data*/, uint32_t* /*len*/) { return RC_E_UNSUPPORTED; }

public:
  bool_t IsSendAllOk() const;
  void SetKeepAlive(uint32_t sec);
  void SetRateControl(uint32_t nKBS, bool_t bSlowDown);

public:
  void SetNetEvent(INetEvent* NetEvent);
  void Set(const char_t* strParam, const void* pData, uint32_t nDataLen);

public:
  static const char_t* m_strAttrUser;
  static const char_t* m_strAttrPWD;

public:
  NetHandlerImpl(IHostContext* pIHostContext, INetEvent* pINetEvent);
  ~NetHandlerImpl();


private:
  void setNetUser(const char_t* strUser, const char_t* strPWD);

  rc_t waitForJobFinish(uint32_t sec = 60 * 60) const;
  rc_t connectToAggregator();
  rc_t disConnectToAggregator();
  void resetJobCount();

  void rateControl(uint32_t nCurrentLen);

  void engineJobNotify();

  static void WINAPI EngineJobNotify(NetHandlerImpl* pNetHandlerImpl);

  IHostContext*   m_pIHostContext;
  DynamicLibrary  m_dyncEngine;
  PFN_TAAPI_CREATEINSTANCE    m_pfnTaApi_CreateInstance;

  IEngine*        m_Taapi_Engine;
  IClient*        m_Taapi_Client;
  INetEvent*      m_pINetEvent;

  bool_t          m_bInit;
  volatile bool_t m_bRunning;

  uint32_t        m_nJobSendCount;
  uint32_t        m_nJobOKCount;
  uint32_t        m_nJobFailedCount;
  atomic_count    m_acJobFinishCount;

private:
  char_t          m_strNetUser[64];
  char_t          m_strNetPWD[64];

private:
  bool_t          m_bSlowDown;

  atomic_count    m_acJobSend;
  atomic_count    m_acJobRecv;
  uint32_t        m_nSendRate;
  uint32_t        m_nLastSendBytes;     // sec

  uint32_t        m_nKeepAlive;
  Time            m_timeLastSend;       // sec
  

private:
  NetworkInterfaceList        m_listNI;
  IPAddressNumber             m_ipTarget;
  MACAddressNumber            m_macTarget;
  
  DISALLOW_COPY_AND_ASSIGN(NetHandlerImpl);
}; // NetHandlerImpl

END_NAMESPACE_COLLECTOR
#endif // NET_HANDLER_IMPL_H_
