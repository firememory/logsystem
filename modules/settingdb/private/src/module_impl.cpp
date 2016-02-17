/*

*/

#include "module_impl.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////
void ModuleImpl::Release() { delete this; }

rc_t ModuleImpl::Init(IAggregatorBase* pIAggregator) {

  ASSERT(FALSE == m_bRegSettingDB);
  if (NULL == pIAggregator) { return RC_S_NULL_VALUE; }
  m_pIAggregator = pIAggregator;

  rc_t rc = m_SettingDB.Init(m_pIAggregator);
  if (RC_S_OK != rc) { return rc; }

  rc = m_pIAggregator->RegisterFunc(&m_SettingDB, strAttrTypeSetting);
  if (RC_S_OK != rc) { return RC_S_FAILED; }

  m_bRegSettingDB = TRUE;
  return RC_S_OK;
}

void ModuleImpl::DeInit() {

  if (NULL == m_pIAggregator) { return; }

  if (TRUE == m_bRegSettingDB) {
    m_pIAggregator->UnRegisterFunc(&m_SettingDB, strAttrTypeLogType);
    m_bRegSettingDB = FALSE;
  }

  m_pIAggregator->Release();
}

void ModuleImpl::TickProc(uint32_t) {
}

void ModuleImpl::CheckPoint() {
}

//////////////////////////////////////////////////////////////////////////
ModuleImpl::ModuleImpl()
  : m_pIAggregator(NULL)
  , m_SettingDB()
  , m_bRegSettingDB(FALSE)
{}

ModuleImpl::~ModuleImpl() {}


IModule* Module_CreateInstance() {
  return new ModuleImpl();
}

END_NAMESPACE_AGGREGATOR
