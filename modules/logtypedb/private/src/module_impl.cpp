/*

*/

#include "module_impl.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////
void ModuleImpl::Release() { delete this; }

rc_t ModuleImpl::Init(IAggregatorBase* pIAggregator) {

  ASSERT(FALSE == m_bRegLogTypeDB);
  if (NULL == pIAggregator) { return RC_S_NULL_VALUE; }
  m_pIAggregator = pIAggregator;

  rc_t rc = m_LogTypeDB.Init(m_pIAggregator);
  if (RC_S_OK != rc) { return rc; }

  rc = m_pIAggregator->RegisterFunc(&m_LogTypeDB, strAttrTypeLogType);
  if (RC_S_OK != rc) { return RC_S_FAILED; }

  m_bRegLogTypeDB = TRUE;
  return RC_S_OK;
}

void ModuleImpl::DeInit() {

  if (NULL == m_pIAggregator) { return; }

  if (TRUE == m_bRegLogTypeDB) {
    m_pIAggregator->UnRegisterFunc(&m_LogTypeDB, strAttrTypeLogType);
    m_bRegLogTypeDB = FALSE;
  }

  m_pIAggregator->Release();
}

void ModuleImpl::TickProc() {
}


//////////////////////////////////////////////////////////////////////////
ModuleImpl::ModuleImpl()
  : m_pIAggregator(NULL)
  , m_LogTypeDB()
  , m_bRegLogTypeDB(FALSE)
{}

ModuleImpl::~ModuleImpl() {}


IModule* Module_CreateInstance() {
  return new ModuleImpl();
}

END_NAMESPACE_AGGREGATOR
