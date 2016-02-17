/*

*/

#include "module_impl.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////
void ModuleImpl::Release() { delete this; }

rc_t ModuleImpl::Init(IAggregatorBase* pIAggregator) {

  ASSERT(FALSE == m_bRegLogTypeTxt);
  ASSERT(FALSE == m_bRegParserTxt);

  if (NULL == pIAggregator) { return RC_S_NULL_VALUE; }
  m_pIAggregator = pIAggregator;

  // LogType
  rc_t rc = m_LogTypeTxt.Init(m_pIAggregator);
  if (RC_S_OK != rc) { return rc; }

  rc = m_pIAggregator->RegisterFunc(&m_LogTypeTxt, strAttrTypeLogType);
  if (RC_S_OK != rc) { return RC_S_FAILED; }

  m_bRegLogTypeTxt = TRUE;

  // Parser
  rc = m_ParserTxt.Init(m_pIAggregator);
  if (RC_S_OK != rc) { return rc; }

  rc = m_pIAggregator->RegisterFunc(&m_ParserTxt, strAttrTypeParser);
  if (RC_S_OK != rc) { return RC_S_FAILED; }

  m_bRegParserTxt = TRUE;
  return RC_S_OK;
}

void ModuleImpl::DeInit() {

  if (NULL == m_pIAggregator) { return; }

  if (TRUE == m_bRegParserTxt) {
    m_pIAggregator->UnRegisterFunc(&m_ParserTxt, strAttrTypeParser);
    m_bRegParserTxt = FALSE;
  }

  if (TRUE == m_bRegLogTypeTxt) {
    m_pIAggregator->UnRegisterFunc(&m_LogTypeTxt, strAttrTypeLogType);
    m_bRegLogTypeTxt = FALSE;
  }

  m_pIAggregator->Release();
}

void ModuleImpl::TickProc() {
}

//////////////////////////////////////////////////////////////////////////
ModuleImpl::ModuleImpl()
  : m_pIAggregator(NULL)
  , m_ParserTxt()
  , m_bRegParserTxt(FALSE)
  , m_LogTypeTxt()
  , m_bRegLogTypeTxt(FALSE)
{}

ModuleImpl::~ModuleImpl() {}


IModule* Module_CreateInstance() {
  return new ModuleImpl();
}

END_NAMESPACE_AGGREGATOR
