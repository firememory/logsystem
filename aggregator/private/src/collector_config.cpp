/*

*/

#include "collector_config.h"

#include "protocol.h"

#include "proto/aggregator.pb.h"
using aggregator::CollectRuleResponse_Rule;
using aggregator::ConfigResponse_Time;
using aggregator::CollectorFileResponse_File;
using aggregator::FileID;

#include "aggregator_impl.h"
#include "getset_param.h"

BEGIN_NAMESPACE_AGGREGATOR

//////////////////////////////////////////////////////////////////////////
// collector config
CollectorConfig::CollectorConfig(const char_t* strCollector)
  : m_strCollector(strCollector)
  , m_pConfigResponse(NULL)
  , m_pCollectorFileResponse(NULL)
  , m_pCollectRuleResponse(NULL)
{

  m_pConfigResponse = new ConfigResponse();
  m_pCollectorFileResponse = new CollectorFileResponse();
  m_pCollectRuleResponse = new CollectRuleResponse();

  defaultConfig();
}

CollectorConfig::~CollectorConfig() {

  if (m_pConfigResponse) { delete m_pConfigResponse; }
}

void CollectorConfig::defaultConfig() {

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

  if (m_pCollectorFileResponse) {
  }

  if (m_pCollectRuleResponse) {
  }
}

void CollectorConfig::SetConfigResponse(const ConfigResponse& configResponse) {

  if (NULL == m_pConfigResponse) { return; }

  m_pConfigResponse->CopyFrom(configResponse);
}
//////////////////////////////////////////////////////////////////////////
typedef GetSetParamImpl<ConfigResponse*>          ConfigGSObject;
template<>
void ConfigGSObject::SetResult(uint32_t row, uint32_t col, const char_t* val, uint32_t len) {

  if (NULL == val || 0 == len) { return; }
  ASSERT(m_r);

  ConfigResponse_Time* pTime = 0 == col ? m_r->add_time() : m_r->mutable_time(row);
  if (NULL == pTime) { return; }

  switch(col) {
    case 0: pTime->set_no(ATOI(val)); break;
    case 1: pTime->set_keepalive(ATOI(val)); break;
    case 2: pTime->set_rate(ATOI(val)); break;
    case 3: pTime->set_start(ATOI(val)); break;
    case 4: pTime->set_end(ATOI(val)); break;
    case 5: pTime->set_steptime(ATOI(val)); break;
  }
}

//////////////////////////////////////////////////////////////////////////
struct GetCollectorFileResult {

  CollectorFileResponse*  pCollectorFileResponse;
  AggregatorImpl*         pAggregatorImpl;
  file_id_t               file_id;


  GetCollectorFileResult(CollectorFileResponse* pCollectorFileResponse, AggregatorImpl* pAggregatorImpl)
    : pCollectorFileResponse(pCollectorFileResponse)
    , pAggregatorImpl(pAggregatorImpl)
    , file_id(IFileNode::INVALID_FILE_ID)
  {}
};

typedef GetSetParamImpl<GetCollectorFileResult*>   CollectorFileGSObject;
template<>
void CollectorFileGSObject::SetResult(uint32_t row, uint32_t col, const char_t* val, uint32_t len) {

  if (NULL == val || 0 == len) { return; }
  ASSERT(m_r);

  CollectorFileResponse_File* pFile = NULL;
  if (0 == col) {

    pFile = m_r->pCollectorFileResponse->add_file();
    if (NULL == pFile) { return; }

    FileID* pFileID = pFile->mutable_id();
    if (NULL == pFileID) { return; }

    m_r->file_id = ATOI(val);
    pFileID->set_id(m_r->file_id); return;
  }
  else {
    pFile = m_r->pCollectorFileResponse->mutable_file(row);
    if (NULL == pFile) { return; }
  }

  switch(col) {
    //case 0: pFileID->set_id(ATOI(val)); break;
    case 1: { 

      file_size_t size = val ? ATOI64(val) : 0;
      // we trust our file system
      AutoRelease<IFileNode*> autoRelFileNode(m_r->pAggregatorImpl->GetFileNode(m_r->file_id));
      if (autoRelFileNode) {
        pFile->set_pos(autoRelFileNode->Size());  
      }
      else { pFile->set_pos(size); }
      break;
    }
    case 2: pFile->set_finish(0 == STRCMP(_STR("FINISH"), val) ? true : false); break;
    case 3: pFile->set_dir(val); break;
    case 4: { pFile->set_name(val); m_r->file_id = IFileNode::INVALID_FILE_ID; break; }
  }
}

//////////////////////////////////////////////////////////////////////////
typedef GetSetParamImpl<CollectRuleResponse*>     CollectRuleGSObject;
void CollectRuleGSObject::SetResult(uint32_t row, uint32_t col, const char_t* val, uint32_t len) {

  if (NULL == val || 0 == len) { return; }
  ASSERT(m_r);

  CollectRuleResponse_Rule* pRule = 0 == col ? m_r->add_rule() : m_r->mutable_rule(row);
  if (NULL == pRule) { return; }

  switch(col) {
    case 0: pRule->set_dir(val); break;
    case 1: pRule->set_exclude(val); break;
    case 2: pRule->set_include(val); break;
  }
}

rc_t CollectorConfig::UpdateConfig(AggregatorImpl* pAggregatorImpl, void* context) {

  AggregatorImpl::CollectClient* pCollectClient = static_cast<AggregatorImpl::CollectClient*>(context);
  if (NULL == pAggregatorImpl || NULL == pCollectClient) { return RC_S_NULL_VALUE; }

  rc_t rc = RC_S_OK;
  if (m_pConfigResponse) {

    m_pConfigResponse->Clear();
    ConfigGSObject configGSObject(m_pConfigResponse, m_strCollector);
    if (RC_S_OK != pAggregatorImpl->GetSetting(&configGSObject, kStrCollectorConfig)) {
      m_pConfigResponse->Clear();
      rc = RC_S_FAILED;
    }
  }

  if (m_pCollectorFileResponse) {

    m_pCollectorFileResponse->Clear();

    GetCollectorFileResult getCollectorFileResult(m_pCollectorFileResponse, pAggregatorImpl);
    CollectorFileGSObject collectorFileGSObject(&getCollectorFileResult, m_strCollector);
    if (RC_S_OK != pAggregatorImpl->GetSetting(&collectorFileGSObject, kStrCollectorFile)) {
      m_pCollectorFileResponse->Clear();
      rc = RC_S_FAILED;
    }
    else {

      pCollectClient->ClearFile();
      int size = m_pCollectorFileResponse->file_size();
      for (int idx = 0; idx < size; ++idx) {
        const CollectorFileResponse_File& file = m_pCollectorFileResponse->file(idx);
        pCollectClient->AddFile(file.id().id());
      }
    }
  }

  if (m_pCollectRuleResponse) {

    m_pCollectRuleResponse->Clear();
    CollectRuleGSObject collectRuleGSObject(m_pCollectRuleResponse, m_strCollector);
    if (RC_S_OK != pAggregatorImpl->GetSetting(&collectRuleGSObject, kStrCollectRule)) {
      m_pCollectRuleResponse->Clear();
      rc = RC_S_FAILED;
    }
  }

  return rc;
}
//////////////////////////////////////////////////////////////////////////
END_NAMESPACE_AGGREGATOR

