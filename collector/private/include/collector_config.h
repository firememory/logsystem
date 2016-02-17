/*

*/

#ifndef COLLECTOR_CONFIG_H_
#define COLLECTOR_CONFIG_H_

#include "collector.h"
#include "host_context.h"

#include "std_list"

namespace aggregator {
class ConfigResponse;
// class CollectRuleResponse;
// class CollectorFileResponse;
}  // namespace aggregator

using aggregator::ConfigResponse;
// using aggregator::CollectorFileResponse;
// using aggregator::CollectRuleResponse;

BEGIN_NAMESPACE_COLLECTOR

USING_AGGREGATOR(IHostContext);

//////////////////////////////////////////////////////////////////////////
class CollectorConfig {
public:
  CollectorConfig(const char_t* strCollector);
  ~CollectorConfig();

public:
  //inline const ConfigResponse* GetConfigResponse() const { return m_pConfigResponse; }
  /*
  inline const CollectorFileResponse* GetCollectorFileResponse() const { return m_pCollectorFileResponse; }
  inline const CollectRuleResponse* GetCollectRuleResponse() const { return m_pCollectRuleResponse; }
  */
  enum { kDefaultSleepTime  = 60 * 60 };
  uint32_t didCollectFile(uint32_t, uint32_t*, uint32_t*, uint32_t*, uint32_t*);
  void SetConfigResponse(const ConfigResponse&, IHostContext*);
private:
  void defaultConfig();

private:
  struct ConfigTime {
    uint32_t no;
    uint32_t keepalive;
    uint32_t rate;
    uint32_t start; 
    uint32_t end; 
    uint32_t steptime; 

    bool operator <(const ConfigTime& other) {
      return start < other.start;
    }
  };

  typedef std::list<ConfigTime>         config_time_list_t;
  config_time_list_t          m_listConfigTime;
  
  //ConfigResponse*             m_pConfigResponse;
  /*
  CollectorFileResponse*      m_pCollectorFileResponse;
  CollectRuleResponse*        m_pCollectRuleResponse;
  */
  //const char_t*               m_strCollector;

  DISALLOW_COPY_AND_ASSIGN(CollectorConfig);
}; // CollectorConfig

END_NAMESPACE_COLLECTOR
#endif // COLLECTOR_CONFIG_H_

