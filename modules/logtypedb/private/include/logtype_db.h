/*

*/

#ifndef LOGTYPE_DB_H_
#define LOGTYPE_DB_H_

//////////////////////////////////////////////////////////////////////////
#include "aggregator_base.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////

class LogTypeDB : public ILogType {

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

public:
  const char_t* LogType() const { return m_strLogType; }
  const char_t* LogType(const IFileNode*, const IMemoryNode*);

public:
  LogTypeDB();
  ~LogTypeDB();

public:
  rc_t Init(IAggregatorBase* pIAggregatorBase);

private:
  IAggregatorBase*      m_pIAggregator;
  static const char_t*  m_strName;
  static const char_t*  m_strLogType;
  static const uuid     m_gcUUID;
  DISALLOW_COPY_AND_ASSIGN(LogTypeDB);
}; // LogTypeDB

END_NAMESPACE_AGGREGATOR
#endif // LOGTYPE_DB_H_

