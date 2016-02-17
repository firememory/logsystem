/*

*/

#ifndef OBJECT_H_
#define OBJECT_H_

#include <config.h>
#include "error.h"

BEGIN_NAMESPACE

//////////////////////////////////////////////////////////////////////////
// must be addref while object was set if no AddRef()
// 
// must be released while object was get
// do not need release while object was param

struct NullClass {};

//////////////////////////////////////////////////////////////////////////
// object
interface IObject {

public:
  // base
  virtual const uuid& GetUUID() const = 0;
  virtual const void* ConstCast(const uuid&) const = 0;
  virtual void* Cast(const uuid&) = 0;

//   virtual const char_t* GetName() const = 0;  
//   virtual const rc_t GetStatus() const = 0;

  // ref count
// protected:
//   virtual void AddRef() = 0;
// 
// public:  
//   virtual void Release() = 0;
}; // IObject
static const uuid kUUID_IObject         = 0x00001001;

interface IResource {
public:
  virtual const rc_t GetName(char_t*, uint32_t*) const = 0;
  virtual const rc_t GetStatus() const = 0;

  virtual const rc_t GetResourceSize(uint64_t*, uint64_t*) const = 0;
  virtual const rc_t GetLastErr(uint32_t*, char_t*, uint32_t*) const = 0;
  virtual const rc_t GetInfo(char_t*, uint32_t*) const = 0;
  virtual const rc_t GetLastAliveTime(uint32_t*) const = 0;
}; // IResource

static const uuid kUUID_IResource       = 0x00001101;

//////////////////////////////////////////////////////////////////////////
// IListener
interface IListener {
public:
  virtual void Release() = 0;

public:
  virtual void Notify() = 0;
}; // IListener


//////////////////////////////////////////////////////////////////////////
/*
class AutoReleaseBase {
}; // AutoReleaseBase
*/
template<typename _Ty>
class AutoRelease {
public:
  void Release() { if (m_handle) { m_handle->Release(); m_handle = NULL; } }

public:
  AutoRelease(AutoRelease& other)
    : m_handle(NULL)
  { exchange(other); }

  AutoRelease(_Ty handle)
    : m_handle(handle)
  {}

  ~AutoRelease() { Release(); }

public:
  inline _Ty operator->() { return m_handle; }
  inline operator _Ty() { return m_handle; }

public:
  void Set(_Ty handle) { Release(); m_handle = handle; }
  inline AutoRelease& operator=(AutoRelease& other) { exchange(other); return *this; }

private:
  void exchange(AutoRelease& other) {

    Release();
    m_handle = other.m_handle;
    other.m_handle = NULL;
  }

private:
  _Ty   m_handle;
  //DISALLOW_COPY_AND_ASSIGN(AutoRelease);
}; // AutoRelease
/*
template<typename _Ty>
class ObjectHolder {

public:
  void Release();

public:
  ObjectHolder(_Ty handle)
    : m_handle(handle)
  {}

  ~ObjectHolder() { Release(); }

public:
  inline operator _Ty() { return m_handle; }

private:
  _Ty               m_handle;
  DISALLOW_COPY_AND_ASSIGN(ObjectHolder);
}; // FileHolder
*/
//////////////////////////////////////////////////////////////////////////
// 
interface IMemoryNode {
public:
  virtual void Release() = 0;
  virtual void AddRef() = 0;

public:
  virtual uint8_t* data() const = 0;
  virtual uint32_t len() const = 0;
}; // IMemoryNode

//////////////////////////////////////////////////////////////////////////
// log
interface ILogWriter {
public:  

  typedef enum log_lv_e {
    LV_STDERR, LV_EMERG, LV_ALERT, LV_CRIT,
    LV_ERR, LV_WARN, LV_NOTICE, LV_INFO, LV_TRACE
  } log_lv_t;

  // 
  interface ILogNode {
  public:
    virtual void Release() = 0;
    virtual void AddRef() = 0; // for Peek
  
  public:
    virtual log_lv_t Type() const = 0;
    virtual uint32_t Time32() const = 0; // milliseconds
    virtual uint64_t Time64() const = 0; // microseconds
    virtual const IMemoryNode* MemNode() const = 0;
  }; // ILogNode

public:
  virtual void Release() = 0;

public:
  virtual rc_t Log(log_lv_t lv, IMemoryNode* pIMemoryNode) = 0;
  virtual rc_t Log(log_lv_t lv, const uint8_t* pLogData, uint32_t len) = 0;
  virtual rc_t Log(log_lv_t lv, const char_t* fmt, ...) = 0;
  virtual rc_t Log(log_lv_t lv, const char_t* fmt, va_list) = 0;
  virtual rc_t Commit(ILogWriter* pILogWriter = NULL) = 0;

public:
  virtual rc_t SetListener(IListener*) = 0;
  virtual ILogNode* Get() = 0;
  virtual ILogNode* Peek() = 0;
}; // ILogWriter

static const char_t*  kStrLOG_LV[] = {
  _STR("STDERR"), _STR("EMERG"), _STR("ALERT"), _STR("CRIT")
  , _STR("ERR"), _STR("WARN"), _STR("NOTICE"), _STR("INFO"), _STR("TRACE")
  , NULL
};

END_NAMESPACE

#endif // OBJECT_H_
