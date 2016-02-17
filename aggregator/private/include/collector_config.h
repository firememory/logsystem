/*

*/

#ifndef COLLECTOR_CONFIG_H_
#define COLLECTOR_CONFIG_H_

#include "aggregator_base.h"

namespace aggregator {
class ConfigResponse;
class CollectRuleResponse;
class CollectorFileResponse;
}  // namespace aggregator

using aggregator::ConfigResponse;
using aggregator::CollectorFileResponse;
using aggregator::CollectRuleResponse;

BEGIN_NAMESPACE_AGGREGATOR
class AggregatorImpl;
//////////////////////////////////////////////////////////////////////////
class CollectorConfig {
public:
  CollectorConfig(const char_t* strCollector);
  ~CollectorConfig();

public:
  rc_t UpdateConfig(AggregatorImpl*, void*);

public:
  inline const ConfigResponse* GetConfigResponse() const { return m_pConfigResponse; }
  inline const CollectorFileResponse* GetCollectorFileResponse() const { return m_pCollectorFileResponse; }
  inline const CollectRuleResponse* GetCollectRuleResponse() const { return m_pCollectRuleResponse; }

  void SetConfigResponse(const ConfigResponse&);

private:
  void defaultConfig();

private:
  ConfigResponse*             m_pConfigResponse;
  CollectorFileResponse*      m_pCollectorFileResponse;
  CollectRuleResponse*        m_pCollectRuleResponse;

  const char_t*               m_strCollector;

  DISALLOW_COPY_AND_ASSIGN(CollectorConfig);
}; // CollectorConfig

END_NAMESPACE_AGGREGATOR
#endif // COLLECTOR_CONFIG_H_

