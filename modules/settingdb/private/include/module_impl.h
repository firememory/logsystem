/*

*/

#ifndef MODULE_IMPL_H_
#define MODULE_IMPL_H_

//////////////////////////////////////////////////////////////////////////
#include "aggregator_base.h"

#include "setting_db.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////

class ModuleImpl : public IModule {

  // IModule
public:
  void Release();

public:
  // so
  rc_t Init(IAggregatorBase*);
  void DeInit();

public:
  const char_t* GetName() const { return SettingDB::GetName(); }
  uint32_t NeedMoreThread() const { return 0; }
  void TickProc(uint32_t);
  void CheckPoint();

public:
  ModuleImpl();
  ~ModuleImpl();

private:
  IAggregatorBase*    m_pIAggregator;

  SettingDB       m_SettingDB;
  bool_t          m_bRegSettingDB;
  DISALLOW_COPY_AND_ASSIGN(ModuleImpl);
}; // ModuleImpl

END_NAMESPACE_AGGREGATOR
#endif // MODULE_IMPL_H_

