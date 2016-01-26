/*

*/

#ifndef NET_HANDLER_H_
#define NET_HANDLER_H_

#include "aggregator.h"

BEGIN_NAMESPACE_AGGREGATOR

interface INetEvent {
public:
  enum net_event_e {CONNECTED = 1, DISCONNECT, RESPONSE_DATA = 10, RESPONSE_STATUS };

public:
  virtual rc_t NetEvent(const void* context, net_event_e type, uint32_t len, const uint8_t* data) = 0;
}; // INetEvent

interface INetHandler {
public:
  enum { IPV4_LEN = 4, MAC_LEN = 6, IPV6_LEN = 16, kMAX_REQ_DATA_SIZE = 60 * 1024 };

public:
  virtual void Release() = 0;

public:
  virtual rc_t InitNet() = 0;
  virtual rc_t DeInitNet() = 0;
  virtual rc_t StartNet() = 0;
  virtual rc_t StopNet() = 0;

public:
  virtual rc_t Open() = 0;
  virtual rc_t Close() = 0;

public:
  virtual uint32_t GetSessionID() const = 0;
  virtual const uint8_t* GetLocalIP(size_t* len) = 0;
  virtual const uint8_t* GetLocalMAC(size_t* len) = 0;

  virtual const uint8_t* GetTargetIP(size_t* len) = 0;
  virtual const uint8_t* GetTargetMAC(size_t* len) = 0;

public:
  virtual rc_t Send(const void* context, const uint8_t* data, uint32_t len) = 0;
  virtual rc_t SendTo(const void* context, uint32_t id, const uint8_t* data, uint32_t len) = 0;

  virtual rc_t Recv(uint8_t* data, uint32_t* len) = 0;
  virtual rc_t RecvForm(uint32_t id, uint8_t* data, uint32_t* len) = 0;

public:
  virtual bool_t IsSendAllOk() const = 0;
  virtual void SetKeepAlive(uint32_t sec) = 0;
  virtual void SetRateControl(uint32_t nKBS, bool_t bSlowDown) = 0;

public:
  virtual void SetNetEvent(INetEvent* NetEvent) = 0;
  virtual void Set(const char_t* strParam, const void* pData, uint32_t nDataLen) = 0;
}; // INetHandler

END_NAMESPACE_AGGREGATOR
#endif // NET_HANDLER_H_
