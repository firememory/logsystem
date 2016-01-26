/*

*/

#include "collector_config.h"

#include "protocol.h"

#include "proto/aggregator.pb.h"
using aggregator::ConfigResponse_Time;
using aggregator::FileID;

#include "time_util.h"

BEGIN_NAMESPACE_COLLECTOR

USING_BASE(DayTime);

//////////////////////////////////////////////////////////////////////////
// collector config
CollectorConfig::CollectorConfig(const char_t* strCollector)
  : m_listConfigTime()
//  : m_strCollector(strCollector)
//  , m_pConfigResponse(NULL)
//   , m_pCollectorFileResponse(NULL)
//   , m_pCollectRuleResponse(NULL)
{

  UNUSED_PARAM(strCollector);
  //m_pConfigResponse = new ConfigResponse();
//   m_pCollectorFileResponse = new CollectorFileResponse();
//   m_pCollectRuleResponse = new CollectRuleResponse();

  defaultConfig();
}

CollectorConfig::~CollectorConfig() {

  //if (m_pConfigResponse) { delete m_pConfigResponse; }
}

void CollectorConfig::defaultConfig() {

  /*
  if (m_pConfigResponse) {

    m_pConfigResponse->Clear();

    // 00 ~ 24  60s
    ConfigResponse_Time* pTime = m_pConfigResponse->add_time();
    if (pTime) {
      pTime->set_keepalive(60);   // 60s
      pTime->set_rate(0);         // no limit
      pTime->set_start(DayTime::START);
      pTime->set_start(DayTime::END);
      pTime->set_steptime(60);
    }
  }
  */
/*
  if (m_pCollectorFileResponse) {
  }

  if (m_pCollectRuleResponse) {
  }
  */
}

void CollectorConfig::SetConfigResponse(const ConfigResponse& configResponse, IHostContext* pIHostContext) {

//   if (NULL == m_pConfigResponse) { return; }
// 
//   m_pConfigResponse->CopyFrom(configResponse);

  #define LOG(lv, ...)          pIHostContext->Log(lv, __VA_ARGS__)

  m_listConfigTime.clear();
  for (int idx = 0; idx < configResponse.time_size(); ++idx) {

    ConfigTime configTime;
    const ConfigResponse_Time& crTime = configResponse.time(idx);
    configTime.no         = crTime.no();
    configTime.keepalive  = crTime.keepalive();
    configTime.rate       = crTime.rate();

    configTime.steptime   = crTime.steptime();
    if (crTime.start() > crTime.end()) {

      configTime.start      = DayTime::to_time(crTime.start());
      configTime.end        = DayTime::END_SEC;
      m_listConfigTime.push_back(configTime);

      LOG(ILogWriter::LV_TRACE, "Collect Time: Start[%u], End[%u], StepTime[%u], KeepAlive[%u], Rate[%u]"
        , crTime.start(), DayTime::END
        , configTime.steptime
        , configTime.keepalive
        , configTime.rate
        );

      configTime.start      = DayTime::START;
      configTime.end        = DayTime::to_time(crTime.end());
      m_listConfigTime.push_back(configTime);

      LOG(ILogWriter::LV_TRACE, "Collect Time: Start[%u], End[%u], StepTime[%u], KeepAlive[%u], Rate[%u]"
        , DayTime::START, crTime.end()
        , configTime.steptime
        , configTime.keepalive
        , configTime.rate
        );
    }
    else {
      configTime.start      = DayTime::to_time(crTime.start());
      configTime.end        = DayTime::to_time(crTime.end());
      m_listConfigTime.push_back(configTime);

      LOG(ILogWriter::LV_TRACE, "Collect Time: Start[%u], End[%u], StepTime[%u], KeepAlive[%u], Rate[%u]"
        , crTime.start(), crTime.end()
        , configTime.steptime
        , configTime.keepalive
        , configTime.rate
        );
    }
  }

  m_listConfigTime.sort();
}

uint32_t CollectorConfig::didCollectFile(uint32_t nLastTime
                                         , uint32_t* nTimeRuleNO
                                         , uint32_t* nStepTime, uint32_t* nKeepAlive, uint32_t* nSendRate) {

  uint32_t now = DayTime::now();
  uint32_t now_day_sec = DayTime::day_sec(now);
  config_time_list_t::iterator it_list, end;
  for (it_list = m_listConfigTime.begin(), end = m_listConfigTime.end(); it_list != end; ++it_list) {

    const ConfigTime& configTime = (*it_list);
    if (now_day_sec > configTime.start && now_day_sec < configTime.end) {

      if (nTimeRuleNO)  { (*nTimeRuleNO) = configTime.no; }
      if (nStepTime)    { (*nStepTime) = configTime.steptime; }
      if (nKeepAlive)   { (*nKeepAlive) = configTime.keepalive; }
      if (nSendRate)    { (*nSendRate) = configTime.rate; }

      if (0 == configTime.steptime) { return kDefaultSleepTime; }
      if (now - nLastTime > configTime.steptime) { return 0; }
      return configTime.steptime - (now - nLastTime);
    }
  }

  // default 
  return kDefaultSleepTime;
}
//////////////////////////////////////////////////////////////////////////
/*
class ConfigObject : public IObject {

  // IObject
public:
  // base
  const uuid& GetUUID() const { return m_uuid; }
  void* Cast(const uuid& id) { 
    if (id == m_uuid) { return m_pObject; }
    return NULL;
  }

  const char_t* GetName() const { return m_strCollector; }
  const rc_t GetStatus() const { return RC_S_OK; }

  // ref count
protected:
  void AddRef() {}

public:  
  void Release() {}

public:
  ConfigObject(void* obj, const uuid& id, const char_t* strCollector)
    : m_pObject(obj)
    , m_uuid(id)
    , m_strCollector(strCollector)
  {
    BZERO_ARR(m_strCollector);
  }

  ~ConfigObject() {}

private:
  void*             m_pObject;
  uuid              m_uuid;
  const char_t*     m_strCollector;
};

rc_t CollectorConfig::UpdateConfig(AggregatorImpl* pAggregatorImpl) {

  if (NULL == pAggregatorImpl) { return RC_S_NULL_VALUE; }

  if (m_pConfigResponse) {

    ConfigObject configObject(m_pConfigResponse, kUUID_ConfigResponse, m_strCollector);
    pAggregatorImpl->GetSetting(&configObject, kStrCollectorConfig);
  }

  if (m_pCollectorFileResponse) {

    ConfigObject configObject(m_pCollectorFileResponse, kUUID_CollectorFileResponse, m_strCollector);
    pAggregatorImpl->GetSetting(&configObject, kStrCollectorFile);
  }

  if (m_pCollectRuleResponse) {

    ConfigObject configObject(m_pCollectRuleResponse, kUUID_CollectRuleResponse, m_strCollector);
    pAggregatorImpl->GetSetting(&configObject, kStrCollectRule);
  }

  return RC_S_OK;
}
*/
//////////////////////////////////////////////////////////////////////////
END_NAMESPACE_COLLECTOR

