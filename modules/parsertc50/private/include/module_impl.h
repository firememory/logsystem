/*

*/

#ifndef MODULE_IMPL_H_
#define MODULE_IMPL_H_

//////////////////////////////////////////////////////////////////////////
#include "aggregator.h"

#include "parser_tc50.h"
#include "logtype_tc50.h"

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
  const char_t* GetName() const { return ParserTC50::GetName(); }
  uint32_t NeedMoreThread() const { return m_nThreadNum; }
  void TickProc(uint32_t);
  void CheckPoint();

public:
  ModuleImpl();
  ~ModuleImpl();

private:
  IAggregatorBase*    m_pIAggregator;

  ParserTC50      m_ParserTC50;
  bool_t          m_bRegParserTC50;

  LogTypeTC50     m_LogTypeTC50;
  bool_t          m_bRegLogTypeTC50;

  uint32_t        m_nThreadNum;
}; // ModuleImpl

END_NAMESPACE_AGGREGATOR
#endif // MODULE_IMPL_H_

