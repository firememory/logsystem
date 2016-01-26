/*

*/

#ifndef LOGTYPE_TXT_H_
#define LOGTYPE_TXT_H_

//////////////////////////////////////////////////////////////////////////
#include "aggregator_base.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////

class LogTypeTxt : public ILogType {

  // IObject
public:
  const uuid& GetUUID() const { return m_gcUUID; }
  const char_t* GetName() const { return m_strName; }
  const rc_t GetStatus() const { return RC_S_OK; }
  const void* ConstCast(const uuid& id) const {
    if (m_gcUUID == id) { return this; }
    else if (kUUID_ILogType == id) { return (ILogType*)(this); }
    return NULL;
  }
  void* Cast(const uuid& id) {
    if (m_gcUUID == id) { return this; }
    else if (kUUID_ILogType == id) { return (ILogType*)(this); }
    return NULL;
  }

  // ref count
protected:
  void AddRef() {}

public:  
  void Release() {}

  // ILogType
public:
  const char_t* LogType() const { return m_strLogType; }
  const char_t* LogType(const IFileNode*, const IMemoryNode*);

public:
  LogTypeTxt();
  ~LogTypeTxt();

public:
  rc_t Init(IAggregatorBase* pIAggregator);

private:
  IAggregatorBase*      m_pIAggregator;
  static const char_t*  m_strName;
  static const char_t*  m_strLogType;
  DISALLOW_COPY_AND_ASSIGN(LogTypeTxt);
}; // LogTypeTxt

END_NAMESPACE_AGGREGATOR
#endif // LOGTYPE_TXT_H_

