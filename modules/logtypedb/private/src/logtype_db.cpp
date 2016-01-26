/*

*/

#include "logtype_db.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////
const char_t* LogTypeDB::LogType(const IFileNode*, const IMemoryNode*) {

  return m_strLogType;
}

//////////////////////////////////////////////////////////////////////////
const char_t* LogTypeDB::m_strLogType   = _STR("BIN");
const char_t* LogTypeDB::m_strName      = _STR("LogTypeDB");
const uuid LogTypeDB::m_gcUUID          = 0x00003029;
//////////////////////////////////////////////////////////////////////////
LogTypeDB::LogTypeDB()
  : m_pIAggregator(NULL)
{}

LogTypeDB::~LogTypeDB() {
}

rc_t LogTypeDB::Init(IAggregatorBase* pIAggregator) {

  if (NULL == pIAggregator) { return RC_S_NULL_VALUE; }

  m_pIAggregator = pIAggregator;
  return RC_S_OK;
}

END_NAMESPACE_AGGREGATOR
