/*

*/

#ifndef HOST_CONTEXT_IMPL_H_
#define HOST_CONTEXT_IMPL_H_

#include "logsystem.h"

#include "host_context.h"

#include <vector>

class CT2EEHandler;

// 
BEGIN_NAMESPACE_LOGSYSTEM

USING_AGGREGATOR(INetEvent);
USING_AGGREGATOR(INetHandler);
USING_AGGREGATOR(IHostContext);

class NetHandlerImpl : public INetHandler {

  // INetHandler
public:
  void Release();

public:
  rc_t InitNet() { return RC_S_OK; }
  rc_t DeInitNet() { return RC_S_OK; }
  rc_t StartNet() { return RC_S_OK; }
  rc_t StopNet() { return RC_S_OK; }

public:
  rc_t Open() { return RC_S_OK; }
  rc_t Close() { return RC_S_OK; }

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
  bool_t IsSendAllOk() const { return TRUE; }
  void SetKeepAlive(uint32_t /*sec*/) {}
  void SetRateControl(uint32_t /*nKBS*/, bool_t /*bSlowDown*/) {}

public:
  void SetNetEvent(INetEvent* /*NetEvent*/) {}
  void Set(const char_t* /*strParam*/, const void* /*pData*/, uint32_t /*nDataLen*/) {}

public:
  NetHandlerImpl(CT2EEHandler* pT2EEHandler, IServer* pIServer,
    ISession* pISession,IConnect* pIConnect,IBusiness* pIBusiness, RAWANSCNTX* pRawAnsCntx);
  ~NetHandlerImpl();

private:
  ISession* m_pISession;
  IConnect* m_pIConnect;
  IBusiness* m_pIBusiness;  
  RAWANSCNTX* m_pRawAnsCntx;

  IServer*		m_pIServer;
  CT2EEHandler* m_pT2EEHandler;

private:
  //#include "net_util.h"
  typedef std::vector<unsigned char> IPAddressNumber;
  typedef IPAddressNumber     MACAddressNumber;

  IPAddressNumber   m_ipLocal;
  MACAddressNumber  m_macLocal;

  IPAddressNumber   m_ipTarget;
  MACAddressNumber  m_macTarget;
}; // NetHandlerImpl

class HostContextImpl : public IHostContext {
public:
  void Release();

public:
  rc_t Log(ILogWriter::log_lv_t lv, const uint8_t* pLogData, uint32_t len);
  rc_t Log(ILogWriter::log_lv_t lv, const char_t* fmt, ...);
  const char_t* GetSystemExeFullPath() const;

public:
  INetHandler* GetNetHandler() { return NULL; }

public:
  HostContextImpl(CT2EEHandler* pT2EEHandler, IServer* pIServer);
  ~HostContextImpl();

public:
  void SetCT2EEHandler(CT2EEHandler* pT2EEHandler) { m_pT2EEHandler = pT2EEHandler; }
  void ReSet(ISession* pISession,IConnect* pIConnect,IBusiness* pIBusiness, RAWANSCNTX* pRawAnsCntx);

private:
  IServer*		  m_pIServer;
  CT2EEHandler* m_pT2EEHandler;
}; // HostContextImpl


END_NAMESPACE_LOGSYSTEM

#endif // HOST_CONTEXT_IMPL_H_
