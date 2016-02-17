/*

*/

#ifndef AGGREGATOR_BASE_H_
#define AGGREGATOR_BASE_H_

//////////////////////////////////////////////////////////////////////////
/*

*/
#include "aggregator.h"
#include "aggregator_export.h"

#include "object.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////
interface IModule;
interface IFileIteratorBase;
interface IHostContext;
interface IGetSetParam;
typedef void       IDBConnection;

interface IAggregatorBase {
public:
  virtual void Release() = 0;

public:
  virtual rc_t RegisterFunc(IObject*, const char_t*) = 0;
  virtual rc_t UnRegisterFunc(IObject*, const char_t*) = 0;

public:
  virtual IFileIteratorBase* GetFileIterator(const char_t* strType, uint32_t nWaitTime) = 0;
  virtual IDBConnection* GetDBConnection(const char_t* strType, uint32_t nWaitTime) = 0;
  virtual ILogWriter* GetLogWriter(uint32_t nWaitTime) = 0;

  virtual const char_t* GetSystemFullPath() = 0;

public:
  virtual rc_t SetSetting(char_t const* name, const IGetSetParam*) = 0;
  virtual rc_t GetSetting(IGetSetParam*, char_t const* name) = 0;

}; // IAggregatorBase

//////////////////////////////////////////////////////////////////////////
static const char_t* strModuleCreateInstance   = _STR("Module_CreateInstance");
typedef IModule* (*PFN_MODULE_CREATE_INSTANCE)();

//////////////////////////////////////////////////////////////////////////
// 
typedef uint32_t                        file_id_t;

interface IFileNode {
public:
  virtual void Release() = 0;
  
public:
  enum { INVALID_FILE_ID = 0 };
  typedef enum STATUS_E_ { PROCESS, FINISH, UNKNOWN } STATUS_e;

public:
  virtual file_id_t ID() const = 0; // Unique
  virtual file_size_t Size() const = 0;
  virtual STATUS_e Status() const = 0;
  virtual const char_t* CheckSum() const = 0;
  virtual const char_t* LocalPath() const = 0;
  virtual const char_t* RemotePath() const = 0;
  virtual const char_t* Collector() const = 0;
  virtual const uint64_t CreateTime() const = 0;

public:
  virtual file_size_t GetNFPos() const = 0;
  virtual void SetID(file_id_t) = 0; // Unique
  virtual void SetSize(file_size_t) = 0; // not real set file size
}; // ILogFileNode

//////////////////////////////////////////////////////////////////////////
// uuid
// system   0x00001000
// moudle   0x00003000
// ISetting     0x00003101
// IParser      0x00003201
// ILogType     0x00003301


interface IModule {
public:
  virtual void Release() = 0;

public:
  // so
  virtual rc_t Init(IAggregatorBase*) = 0;
  virtual void DeInit() = 0;

public:
  virtual const char_t* GetName() const = 0;
  virtual uint32_t NeedMoreThread() const = 0;
  virtual void TickProc(uint32_t) = 0;
  virtual void CheckPoint() = 0;
}; // Module


interface IGetSetParam {
public:
  // idx [1, )
  virtual const char_t* GetCondition(uint32_t idx, uint32_t* len) const = 0;

public: // Result
  virtual void SetResult(uint32_t row, uint32_t col, const char_t*, uint32_t) = 0;
}; // IGetSetParam

// val protobuf
interface ISetting : public IObject, public IResource {
public:  
  virtual void Release() = 0;

public:
  virtual rc_t Set(char_t const* name, const IGetSetParam*) = 0;
  virtual rc_t Get(IGetSetParam*, char_t const* name) = 0;
}; // ISetting
static const uuid kUUID_ISetting        = 0x00003101;

interface IParser : public IObject , public IResource{
public:  
  virtual void Release() = 0;

public:
  // work
  virtual rc_t Start() = 0;
  virtual void Stop() = 0;
}; // IParser
static const uuid kUUID_IParser         = 0x00003201;

interface ILogType : public IObject, public IResource {
public:  
  virtual void Release() = 0;

public:
  virtual const char_t* LogType() const = 0;

  // return 
  //  NULL        unknown need next time
  //  NULL_STR    unsupport
  virtual const char_t* LogType(const IFileNode*, const IMemoryNode*) = 0;
}; // ISetting
static const uuid kUUID_ILogType        = 0x00003301;

//////////////////////////////////////////////////////////////////////////

interface IFileIteratorBase {
public:
  virtual void Release() = 0;

public:
  /*
    synchronous method 

    return 
      RC_S_CLOSED   ==> this file is finish
      RC_S_OK       ==> normal
      ...           ==> break
  */
  class IFileIteratorCallBack {
  public:
    virtual rc_t Begin(IFileIteratorBase*) = 0;
    virtual rc_t FileProc(IFileIteratorBase*, const IFileNode*) = 0;
    virtual rc_t End() = 0;
  }; // IFileIteratorCallBack

  virtual rc_t file_iterate(IFileIteratorCallBack*) = 0;

public:
  virtual IMemoryNode* GetFileData(file_id_t, file_size_t* pos) = 0;
}; // IFileIteratorBase

//////////////////////////////////////////////////////////////////////////
// Module
static const char_t* strAttrType        = _STR("Type");
static const char_t* strAttrTypeSetting = _STR("Setting");
static const char_t* strAttrTypeParser  = _STR("Parser");
static const char_t* strAttrTypeLogType = _STR("LogType");

static const uint32_t kMAX_LOGTYPE_LEN  = 64;
static const uint32_t kMAX_NAME_LEN     = 64;

static const uint32_t kCloseFileTimeOut = 60 * 60; // 1h
static const uint32_t kKeepDBConnTimeOut= 60 * 60; // 1h
static const uint32_t kKeepClientTimeOut= 60 * 60; // 1h
//////////////////////////////////////////////////////////////////////////
// protocol id
static const uint16_t ProtocolID_LSA    = 9888;

//////////////////////////////////////////////////////////////////////////
// interface ILogDBParam : public IObject {
// public:
//   virtual const ILogWriter::log_lv_t lv() const = 0;
//   virtual const char_t* from() const = 0;
//   virtual const uint8_t* data(uint32_t* len) const = 0;
//   virtual const uint8_t* content(uint32_t* len) const = 0;
// }; // ILogDBParam
// static const uuid kUUID_ILogDBParam     = 0x00003193;


END_NAMESPACE_AGGREGATOR

#endif // AGGREGATOR_BASE_H_
