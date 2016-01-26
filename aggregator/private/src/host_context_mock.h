/*

*/

#include "host_context.h"


BEGIN_NAMESPACE_AGGREGATOR

class NetHandlerMock : public INetHandler {

  // INetHandler
public:
  void Release() {}

public:
  rc_t InitNet() { return RC_S_OK; }
  rc_t DeInitNet() { return RC_S_OK; }
  rc_t StartNet() { return RC_S_OK; }
  rc_t StopNet() { return RC_S_OK; }

public:
  rc_t Open() { return RC_S_OK; }
  rc_t Close() { return RC_S_OK; }

public:
  virtual uint32_t GetSessionID() const {;}
  virtual bool_t IsSendAllOk() const{;}
  virtual void SetKeepAlive(uint32_t sec){;};
  virtual void SetRateControl(uint32_t nKBS, bool_t bSlowDown){;};

  uint32_t GetUniqueID() const { return m_id; }
  const uint8_t* GetLocalIP(size_t* len) { if (len) { (*len) = IPV4_LEN; } return m_gcIP; }
  const uint8_t* GetLocalMAC(size_t* len) { if (len) { (*len) = MAC_LEN; } return m_gcMAC; }

  const uint8_t* GetTargetIP(size_t* len) { if (len) { (*len) = IPV4_LEN; } return m_gcIP; }
  const uint8_t* GetTargetMAC(size_t* len) { if (len) { (*len) = MAC_LEN; } return m_gcMAC; }


public:
  rc_t Send(const void* /*context*/, const uint8_t* /*data*/, uint32_t /*len*/) { return RC_S_OK; }
  rc_t SendTo(const void* /*context*/, uint32_t /*id*/, const uint8_t* /*data*/, uint32_t /*len*/) { return RC_E_UNSUPPORTED; }

  rc_t Recv(uint8_t* /*data*/, uint32_t* /*len*/) { return RC_E_UNSUPPORTED; }
  rc_t RecvForm(uint32_t /*id*/, uint8_t* /*data*/, uint32_t* /*len*/) { return RC_E_UNSUPPORTED; }

public:
  void SetNetEvent(INetEvent* /*NetEvent*/) {}
  void Set(const char_t* /*strParam*/, const void* /*pData*/, uint32_t /*nDataLen*/) {}

public:
  NetHandlerMock(uint32_t id) : m_id(id) {}
  ~NetHandlerMock() {}

private:
  uint32_t                    m_id;
  static const uint8_t        m_gcIP[INetHandler::IPV6_LEN + 1];
  static const uint8_t        m_gcMAC[INetHandler::MAC_LEN + 1];
}; // NetHandlerMock

const uint8_t NetHandlerMock::m_gcIP[INetHandler::IPV6_LEN + 1]      = {0x7F, 0x00, 0x00, 0x01, 0x00};
const uint8_t NetHandlerMock::m_gcMAC[INetHandler::MAC_LEN + 1]      = {0x00, 0xE0, 0x4C, 0x6F, 0x9C, 0x5E, 0x00};

#include <direct.h>

class HostContextMock : public IHostContext {
public:
  void Release() {}

public:
  rc_t Log(ILogWriter::log_lv_t lv, const uint8_t* pLogData, uint32_t len) {

    UNUSED_PARAM(lv);
    UNUSED_PARAM(len);
    PRINTF("%s", pLogData);
    return RC_S_OK;
  }

  rc_t Log(ILogWriter::log_lv_t lv, const char_t* fmt, ...) {

    UNUSED_PARAM(lv);

    va_list arglist;
    va_start(arglist, fmt);
    PRINTF(fmt, arglist);
    va_end(arglist);

    return RC_S_OK;
  }
  const char_t* GetSystemExeFullPath() const { return m_strExeFullPath; }

public:
  INetHandler* GetNetHandler() { return m_pINetHandler; }

public:
  HostContextMock(INetHandler* pINetHandler) : m_pINetHandler(pINetHandler) {

    BZERO_ARR(m_strExeFullPath);
    if (_getcwd(m_strExeFullPath, sizeof(m_strExeFullPath))) {
      STRCAT(m_strExeFullPath, sizeof(m_strExeFullPath), STR_DIR_SEP);
    }
    else {  
      STRCPY(m_strExeFullPath, sizeof(m_strExeFullPath), NULL_STR);
    }
  }
  ~HostContextMock() {}

public:
  void ReSet() {}

private:
  INetHandler* m_pINetHandler;
  char_t m_strExeFullPath[kMAX_PATH];
}; // HostContextMock

END_NAMESPACE_AGGREGATOR
