/*

*/

#include "logtype_txt.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////

const char_t* LogTypeTxt::LogType(const IFileNode* pIFileNode, const IMemoryNode* pIMemoryNode) {

  if (NULL == pIFileNode || NULL == pIMemoryNode) { return NULL; }

  uint32_t len = pIMemoryNode->len();
  const uint8_t* data = pIMemoryNode->data();

  UNUSED_LOCAL_VARIABLE(len);
  UNUSED_LOCAL_VARIABLE(data);
  return m_strLogType;
}

//////////////////////////////////////////////////////////////////////////
const char_t* LogTypeTxt::m_strLogType  = _STR("TXT");
const char_t* LogTypeTxt::m_strName     = _STR("LogTypeTxt");
const uuid LogTypeTxt::m_gcUUID         = 0x00003211;
//////////////////////////////////////////////////////////////////////////
LogTypeTxt::LogTypeTxt()
  : m_pIAggregator(NULL)
{}

LogTypeTxt::~LogTypeTxt() {
}

rc_t LogTypeTxt::Init(IAggregatorBase* pIAggregator) {

  if (NULL == pIAggregator) { return RC_S_NULL_VALUE; }

  m_pIAggregator = pIAggregator;
  return RC_S_OK;
}

END_NAMESPACE_AGGREGATOR
