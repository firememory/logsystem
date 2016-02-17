/*

*/

#ifndef LOGTYPE_TC50_H_
#define LOGTYPE_TC50_H_

//////////////////////////////////////////////////////////////////////////
#include "aggregator_base.h"

#include "std_list"

#include "regex.hpp"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////

struct LogTypeRule {
  char_t    strDir[kMAX_PATH + 1];
  char_t    strName[kMAX_NAME_LEN + 1];
  uint32_t  judge_rule;

  RegEx     regexDir;
  RegEx     regexName;

  LogTypeRule()
    : judge_rule(0)
    , regexDir(0)
    , regexName(0)
  {
    BZERO_ARR(strDir);
    BZERO_ARR(strName);
  }
}; // LogTypeRule


class LogTypeTC50 : public ILogType {

  // IObject
public:
  const uuid& GetUUID() const { return m_gcUUID; }

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

  // IResource
public:
  const rc_t GetName(char_t*, uint32_t*) const;
  const rc_t GetStatus() const;

  const rc_t GetResourceSize(uint64_t*, uint64_t*) const;
  const rc_t GetLastErr(uint32_t*, char_t*, uint32_t*) const;
  const rc_t GetInfo(char_t*, uint32_t*) const;
  const rc_t GetLastAliveTime(uint32_t*) const;

  // ILogType
public:  
  void Release() {}

public:
  const char_t* LogType() const { return StrLogType(); }
  const char_t* LogType(const IFileNode*, const IMemoryNode*);

public:
  static const char_t* StrLogType() { return m_strLogType; }

public:
  LogTypeTC50();
  ~LogTypeTC50();

public:
  rc_t Init(IAggregatorBase* pIAggregator);

private:
  rc_t loadLogTypeRule();
  const char_t* getFileName(const char_t*);

public:
  typedef std::list<LogTypeRule>    log_type_rule_list_t;
  log_type_rule_list_t        m_listLogTypeRule;

private:
  uint32_t              m_nCountType;
  uint32_t              m_timeAlive;

private:
  IAggregatorBase*      m_pIAggregator;
  static const char_t*  m_strName;
  static const char_t*  m_strLogType;
  static const uuid     m_gcUUID;
  DISALLOW_COPY_AND_ASSIGN(LogTypeTC50);
}; // LogTypeTC50


END_NAMESPACE_AGGREGATOR
#endif // LOGTYPE_TC50_H_

