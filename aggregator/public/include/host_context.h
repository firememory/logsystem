/*

*/

#ifndef HOST_CONTEXT_H_
#define HOST_CONTEXT_H_

#include "net_handler.h"
#include "object.h"

BEGIN_NAMESPACE_AGGREGATOR

interface IHostContext {
public:
  virtual void Release() = 0;

public:
  virtual rc_t Log(ILogWriter::log_lv_t lv, const uint8_t* pLogData, uint32_t len) = 0;
  virtual rc_t Log(ILogWriter::log_lv_t lv, const char_t* fmt, ...) = 0;
  virtual const char_t* GetSystemExeFullPath() const = 0;

public:
  virtual INetHandler* GetNetHandler() = 0;
}; // IHostContext

interface IInterrupt {
public:
  virtual bool_t didInterrupt() = 0;

public:
  virtual uint32_t InterruptNO() = 0;
  virtual const char_t* InterruptInfo() const = 0;
}; // IInterrupt

END_NAMESPACE_AGGREGATOR

#endif // HOST_CONTEXT_H_
