/*

*/

#include "module_impl.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////
void ModuleImpl::Release() { delete this; }

rc_t ModuleImpl::Init(IAggregatorBase* pIAggregator) {

  ASSERT(FALSE == m_bRegLogTypeTC50);
  ASSERT(FALSE == m_bRegParserTC50);

  if (NULL == pIAggregator) { return RC_S_NULL_VALUE; }
  m_pIAggregator = pIAggregator;

  // LogType
  rc_t rc = m_LogTypeTC50.Init(m_pIAggregator);
  if (RC_S_OK != rc) { return rc; }

  rc = m_pIAggregator->RegisterFunc(&m_LogTypeTC50, strAttrTypeLogType);
  if (RC_S_OK != rc) { return RC_S_FAILED; }

  m_bRegLogTypeTC50 = TRUE;

  // Parser
  rc = m_ParserTC50.Init(m_pIAggregator);
  if (RC_S_OK != rc) { return rc; }

  rc = m_pIAggregator->RegisterFunc(&m_ParserTC50, strAttrTypeParser);
  if (RC_S_OK != rc) { return RC_S_FAILED; }

  m_bRegParserTC50 = TRUE;

  m_nThreadNum = m_ParserTC50.GetThreadNum();

  return RC_S_OK;
}

void ModuleImpl::DeInit() {

  if (NULL == m_pIAggregator) { return; }

  if (TRUE == m_bRegParserTC50) {
    m_pIAggregator->UnRegisterFunc(&m_ParserTC50, strAttrTypeParser);
    m_bRegParserTC50 = FALSE;
  }

  if (TRUE == m_bRegLogTypeTC50) {
    m_pIAggregator->UnRegisterFunc(&m_LogTypeTC50, strAttrTypeLogType);
    m_bRegLogTypeTC50 = FALSE;
  }

  m_pIAggregator->Release();
}

void ModuleImpl::TickProc(uint32_t idx) {

  if (idx) {
    ASSERT(m_bRegParserTC50);
    m_ParserTC50.ThreadProc(idx - 1);
  }
  else {
    if (TRUE == m_bRegParserTC50) {
      m_ParserTC50.CheckPoint();
    }
  }
}

void ModuleImpl::CheckPoint() {
  ASSERT(m_bRegParserTC50);
  m_ParserTC50.CheckPoint();
}

//////////////////////////////////////////////////////////////////////////
ModuleImpl::ModuleImpl()
  : m_pIAggregator(NULL)
  , m_ParserTC50()
  , m_bRegParserTC50(FALSE)
  , m_LogTypeTC50()
  , m_bRegLogTypeTC50(FALSE)
  , m_nThreadNum(0)
{}

ModuleImpl::~ModuleImpl() {}


IModule* Module_CreateInstance() {
  return new ModuleImpl();
}

END_NAMESPACE_AGGREGATOR
