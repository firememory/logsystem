/*

*/

#ifndef MODULE_IMPL_H_
#define MODULE_IMPL_H_

//////////////////////////////////////////////////////////////////////////
#include "aggregator.h"

#include "parser_txt.h"
#include "logtype_txt.h"

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

  ParserTxt       m_ParserTxt;
  bool_t          m_bRegParserTxt;

  LogTypeTxt      m_LogTypeTxt;
  bool_t          m_bRegLogTypeTxt;
}; // ModuleImpl

END_NAMESPACE_AGGREGATOR
#endif // MODULE_IMPL_H_

