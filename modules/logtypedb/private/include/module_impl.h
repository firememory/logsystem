/*

*/

#ifndef MODULE_IMPL_H_
#define MODULE_IMPL_H_

//////////////////////////////////////////////////////////////////////////
#include "aggregator.h"

#include "logtype_db.h"

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
  bool_t NeedMoreThread() const { return FALSE; }
  void TickProc();

public:
  ModuleImpl();
  ~ModuleImpl();

private:
  IAggregatorBase*    m_pIAggregator;

  LogTypeDB       m_LogTypeDB;
  bool_t          m_bRegLogTypeDB;

  DISALLOW_COPY_AND_ASSIGN(ModuleImpl);
}; // ModuleImpl

END_NAMESPACE_AGGREGATOR
#endif // MODULE_IMPL_H_

