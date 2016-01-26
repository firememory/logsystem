/*

*/

#include "log_collector_impl.h"

#include "thread.h"

#include "protocol.h"

#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "proto/aggregator.pb.h"
using namespace aggregator;

#ifdef _MSC_VER
# pragma warning(disable: 4482)
#endif // _MSC_VER


BEGIN_NAMESPACE_COLLECTOR
//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
// protobuf
namespace {

  using namespace google::protobuf;
  // Force shutdown at process exit so that we can test for memory leaks.  To
  // actually check for leaks, I suggest using the heap checker included with
  // google-perftools.  Set it to "draconian" mode to ensure that every last
  // call to malloc() has a corresponding free().
  struct ForceShutdown {
    ~ForceShutdown() {
      ShutdownProtobufLibrary();
    }
  } force_shutdown;

}  // namespace

//////////////////////////////////////////////////////////////////////////
// 
static const uint32_t kdefaultPacketMemorySize    = 1* 1024 * 1024;
class RequestAndResponse {

public:
  void Release() { delete this; }
  static RequestAndResponse* CreateInstance(uint32_t nUniqueID = 1) { return new RequestAndResponse(nUniqueID); }

private:
  RequestAndResponse(uint32_t)
    : m_CollectorRequest()
    , m_pRequestMem((uint8_t*)(malloc(kdefaultPacketMemorySize)))
    , m_nRequestSize(0)
    , m_session_id(0)
  {}

  ~RequestAndResponse() {
    if (m_pRequestMem) { free(m_pRequestMem); }
  }

  // response
public:
  inline AggregatorResponse& GetAggregatorResponse() { return m_AggregatorResponse; }

  inline const LogonResponse& GetLogonResponse() { return m_AggregatorResponse.logon_response(); }

  inline const KeepAliveResponse& GetKeepAliveResponse() { return m_AggregatorResponse.keepalive_response(); }
  inline const LogResponse& GetLogResponse() { return m_AggregatorResponse.log_response(); }

  inline const ConfigResponse& GetConfigResponse() { return m_AggregatorResponse.config_response(); }
  inline const CheckSumResponse& GetCheckSumResponse() { return m_AggregatorResponse.checksum_response(); }

  inline const CollectorFileResponse& GetCollectorFileResponse() { return m_AggregatorResponse.collectorfile_response(); }
  inline const CollectRuleResponse& GetCollectRuleResponse() { return m_AggregatorResponse.collectrule_response(); }

  inline const FileResponse& GetFileResponse() { return m_AggregatorResponse.file_response(); }
  inline const FileDataResponse& GetFileDataResponse() { return m_AggregatorResponse.filedata_response(); }

  inline const InstructResponse& GetInstructResponse() { return m_AggregatorResponse.instruct_response(); }

private:
  AggregatorResponse        m_AggregatorResponse;

  // request
public:
  //CollectorRequest& GetCollectorRequest() { m_CollectorRequest.Clear(); return m_CollectorRequest; }

  inline LogonRequest* GetLogonRequest() { return m_CollectorRequest.mutable_logon_request(); }
  inline LogoutRequest* GetLogoutRequest() { return m_CollectorRequest.mutable_logout_request(); }
  inline KeepAliveRequest* GetKeepAliveRequest() { return m_CollectorRequest.mutable_keepalive_request(); }
  inline LogRequest* GetLogRequest() { return m_CollectorRequest.mutable_log_request(); }

  inline ConfigRequest* GetConfigRequest() { return m_CollectorRequest.mutable_config_request(); }

  inline CollectorFileRequest* GetCollectorFileRequest() { return m_CollectorRequest.mutable_collectorfile_request(); }
  inline CollectRuleRequest* GetCollectRuleRequest() { return m_CollectorRequest.mutable_collectrule_request(); }

  inline FileRequest* GetFileRequest() { return m_CollectorRequest.mutable_file_request(); }
  inline FileDataRequest* GetFileDataRequest() { return m_CollectorRequest.mutable_filedata_request(); }
  inline CheckSumRequest* GetCheckSumRequest() { return m_CollectorRequest.mutable_checksum_request(); }

  rc_t SerializeRequest() {

    m_CollectorRequest.set_session_id(m_session_id);
    ASSERT(m_CollectorRequest.IsInitialized());
    google::protobuf::io::ArrayOutputStream zcos(m_pRequestMem, kdefaultPacketMemorySize);
    if (!m_CollectorRequest.SerializeToZeroCopyStream(&zcos)) { return RC_S_FAILED; }

    m_nRequestSize = (uint32_t)(zcos.ByteCount());
    m_CollectorRequest.Clear();
    return RC_S_OK;    
  }

  void SetSessionID(uint64_t session_id) { m_session_id = session_id; }

  inline const uint8_t* GetRequestData() { ASSERT(m_pRequestMem); return m_pRequestMem; }
  inline const uint32_t GetRequestSize() { ASSERT(m_pRequestMem); return m_nRequestSize; }

private:
  CollectorRequest            m_CollectorRequest;
  uint8_t*                    m_pRequestMem;
  uint32_t                    m_nRequestSize;
  uint64_t                    m_session_id;

  DISALLOW_COPY_AND_ASSIGN(RequestAndResponse);
}; // RequestAndResponse

typedef RequestAndResponse      Response;
typedef RequestAndResponse      Request;

//////////////////////////////////////////////////////////////////////////
rc_t LogCollectorImpl::packetProc(uint32_t nUniqueID, uint32_t len, const uint8_t* data) {

  Response* pResponse = getResponse(nUniqueID);
  if (NULL == pResponse) { return RC_S_NULL_VALUE; }

  m_tcKeepAlive.UpdateTime();

  AggregatorResponse& aggregatorResponse = pResponse->GetAggregatorResponse();
  if (false == aggregatorResponse.ParseFromArray(data, len)) { return RC_S_FAILED; }

  rc_t rc = RC_S_OK;
  if (aggregatorResponse.has_logon_response()) { rc = packetProcLogon(pResponse); }
  if (RC_S_OK != rc) { return rc; }

  if (aggregatorResponse.has_keepalive_response()) { rc = packetProcKeepAlive(pResponse); }
  if (RC_S_OK != rc) { return rc; }

  if (aggregatorResponse.has_log_response()) { rc = packetProcLog(pResponse); }
  if (RC_S_OK != rc) { return rc; }
  
  if (aggregatorResponse.has_config_response()) { rc = packetProcConfig(pResponse); }
  if (RC_S_OK != rc) { return rc; }

  if (aggregatorResponse.has_collectorfile_response()) { rc = packetProcCollectorFile(pResponse); }
  if (RC_S_OK != rc) { return rc; }

  if (aggregatorResponse.has_collectrule_response()) { rc = packetProcCollectRule(pResponse); }
  if (RC_S_OK != rc) { return rc; }

  if (aggregatorResponse.has_file_response()) { rc = packetProcFile(pResponse); }
  if (RC_S_OK != rc) { return rc; }

  if (aggregatorResponse.has_filedata_response()) { rc = packetProcFileData(pResponse); }
  if (RC_S_OK != rc) { return rc; }

  if (aggregatorResponse.has_checksum_response()) { rc = packetProcCheckSum(pResponse); }
  if (RC_S_OK != rc) { return rc; }

  if (aggregatorResponse.has_instruct_response()) { rc = packetProcInstruct(pResponse); }
  if (RC_S_OK != rc) { return rc; }

  return RC_S_OK;
}

rc_t LogCollectorImpl::packetProcLogon(Response* pResponse) {

  ASSERT(pResponse);
  const LogonResponse& logonResponse = pResponse->GetLogonResponse();
  pResponse->SetSessionID(logonResponse.session_id());

  if (logonResponse.has_info()) {
    setAggregator(logonResponse.info().c_str());
  }

  // send request
  m_nSyncPos = 0;
  return RC_S_OK;
}

rc_t LogCollectorImpl::packetProcLogout(Response* pResponse) {

  UNUSED_PARAM(pResponse);
  setLogon(FALSE);
  return RC_S_CLOSED;
}

rc_t LogCollectorImpl::packetProcKeepAlive(Response* pResponse) {

  UNUSED_PARAM(pResponse);

  const KeepAliveResponse& keepAliveResponse = pResponse->GetKeepAliveResponse();
  m_nAggSyncPos = keepAliveResponse.sync_pos();
  
  return RC_S_OK;
}

rc_t LogCollectorImpl::packetProcLog(Response* pResponse) {

  UNUSED_PARAM(pResponse);

  return RC_S_OK;
}

rc_t LogCollectorImpl::packetProcConfig(Response* pResponse) {

  ASSERT(pResponse);
  const ConfigResponse& configResponse = pResponse->GetConfigResponse();

  // config
  CollectorConfig& collectorConfig = getCollectorConfig();
  collectorConfig.SetConfigResponse(configResponse, m_autoRelIHostContext);
  return RC_S_OK;
}

rc_t LogCollectorImpl::packetProcCheckSum(Response* pResponse) {

  UNUSED_PARAM(pResponse);

  return RC_S_OK;
}

rc_t LogCollectorImpl::packetProcCollectorFile(Response* pResponse) {

  ASSERT(pResponse);
  const CollectorFileResponse& collectorFileResponse = pResponse->GetCollectorFileResponse();

  clearFileNode();
  for (int idx = 0; idx < collectorFileResponse.file_size(); ++idx) {

    const CollectorFileResponse_File& collectorFile = collectorFileResponse.file(idx);
    const FileID& fileID = collectorFile.id();
    addFileNode(fileID.id(), collectorFile.pos(), collectorFile.finish(),
      collectorFile.dir().c_str(), collectorFile.name().c_str());
  }
  return RC_S_OK;
}

rc_t LogCollectorImpl::packetProcCollectRule(Response* pResponse) {

  ASSERT(pResponse);
  const CollectRuleResponse& collectRuleResponse = pResponse->GetCollectRuleResponse();

  clearCollectRule();
  for (int idx = 0; idx < collectRuleResponse.rule_size(); ++idx) {

    const CollectRuleResponse_Rule& collectRule = collectRuleResponse.rule(idx);
    addCollectRule(collectRule.dir().c_str(), collectRule.exclude().c_str(), collectRule.include().c_str());
  }

  setLogon(TRUE);
  return RC_S_OK;
}

rc_t LogCollectorImpl::packetProcInstruct(Response* pResponse) {

  ASSERT(pResponse);
  const InstructResponse& instructResponse = pResponse->GetInstructResponse();

  LOG(ILogWriter::LV_INFO, "Recv Instruct");

  if (InstructResponse_EncodeType_CollectNow & instructResponse.instruction()) { fileCollect(); }
  if (InstructResponse_EncodeType_SlowDown & instructResponse.instruction()) { slowDown(); }
  return RC_S_OK;
}

rc_t LogCollectorImpl::packetProcFile(Response* pResponse) {

  ASSERT(pResponse);
  const FileResponse& fileResponse = pResponse->GetFileResponse();

  for (int idx = 0; idx < fileResponse.file_size(); ++idx) {
  
    const FileResponse_File& file = fileResponse.file(idx);
    const FileID& fileID = file.id();
    addFileNode(fileID.id(), 0, FALSE, file.dir().c_str(), file.name().c_str());
  }
  return RC_S_OK;
}

rc_t LogCollectorImpl::packetProcFileData(Response* pResponse) {

  ASSERT(pResponse);
  const FileDataResponse& fileDataResponse = pResponse->GetFileDataResponse();
  for (int idx = 0; idx < fileDataResponse.filedata_size(); ++idx) {

    const FileDataResponse_FileData& fileData = fileDataResponse.filedata(idx);

    const FileID& fileID = fileData.id();
    updateFileNode(fileID.id(), fileData.size());

    if (fileData.has_checksum()) { checkSumFile(fileID.id()); }
    m_bPackFileData = TRUE;
  }
  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
rc_t LogCollectorImpl::reqLogon() {

  Request* pRequest = getRequest(0);
  if (NULL == pRequest) { return RC_S_NULL_VALUE; }

  // logon
  LogonRequest* pLogonRequest = pRequest->GetLogonRequest();
  if (NULL == pLogonRequest) { return RC_E_NOMEM; }

  USING_AGGREGATOR(kProtocolVer);

  pLogonRequest->set_ver(kProtocolVer);
  pLogonRequest->set_name(m_strCollector);

  if (RC_S_OK != pRequest->SerializeRequest()) { return RC_S_FAILED; }
  return m_autoRelINetHandler->Send(this, pRequest->GetRequestData(), pRequest->GetRequestSize());
}

rc_t LogCollectorImpl::reqLogout(const char_t* strMsg) {

  Request* pRequest = getRequest(0);
  if (NULL == pRequest) { return RC_S_NULL_VALUE; }

  // logout
  LogoutRequest* pLogoutRequest = pRequest->GetLogoutRequest();
  if (NULL == pLogoutRequest) { return RC_E_NOMEM; }

  if (strMsg) { pLogoutRequest->set_msg(strMsg); }

  if (RC_S_OK != pRequest->SerializeRequest()) { return RC_S_FAILED; }
  return m_autoRelINetHandler->Send(this, pRequest->GetRequestData(), pRequest->GetRequestSize());
}

rc_t LogCollectorImpl::reqKeepAlive() {

  Request* pRequest = getRequest(0);
  if (NULL == pRequest) { return RC_S_NULL_VALUE; }

  // keepalive
  KeepAliveRequest* pKeepAliveRequest = pRequest->GetKeepAliveRequest();
  if (NULL == pKeepAliveRequest) { return RC_E_NOMEM; }

  pKeepAliveRequest->set_sync_pos(m_nSyncPos);

  if (RC_S_OK != pRequest->SerializeRequest()) { return RC_S_FAILED; }
  if (RC_S_OK != m_autoRelINetHandler->Send(this, pRequest->GetRequestData(), pRequest->GetRequestSize())) { return RC_S_FAILED; }

  m_tcKeepAlive.UpdateTime();
  return RC_S_OK;
}

rc_t LogCollectorImpl::reqLog(uint32_t type, const char_t* strExtern, uint8_t* data, uint32_t len) {

  if (NULL == strExtern) { return RC_S_NULL_VALUE; }

  Request* pRequest = getRequest(0);
  if (NULL == pRequest) { return RC_S_NULL_VALUE; }

  // log
  LogRequest* pLogRequest = pRequest->GetLogRequest();
  if (NULL == pLogRequest) { return RC_E_NOMEM; }

  pLogRequest->set_type(type);
  pLogRequest->set_data(strExtern);

  if (data && len) { pLogRequest->set_content(data, len); }

  if (RC_S_OK != pRequest->SerializeRequest()) { return RC_S_FAILED; }
  return m_autoRelINetHandler->Send(this, pRequest->GetRequestData(), pRequest->GetRequestSize());
}

rc_t LogCollectorImpl::reqAllConfig() {

  // send request
  rc_t rc = reqConfig(FALSE);
  if (RC_S_OK != rc) { return RC_S_FAILED; }

  rc = reqCollectorFile(FALSE);
  if (RC_S_OK != rc) { return RC_S_FAILED; }

  rc = reqCollectRule(TRUE);
  if (RC_S_OK != rc) { return RC_S_FAILED; }

  return RC_S_OK;
}

rc_t LogCollectorImpl::reqConfig(bool_t bSend) {

  Request* pRequest = getRequest(0);
  if (NULL == pRequest) { return RC_S_NULL_VALUE; }

  // config
  ConfigRequest* pConfigRequest = pRequest->GetConfigRequest();
  if (NULL == pConfigRequest) { return RC_E_NOMEM; }

  if (TRUE == bSend) {
    if (RC_S_OK != pRequest->SerializeRequest()) { return RC_S_FAILED; }
    return m_autoRelINetHandler->Send(this, pRequest->GetRequestData(), pRequest->GetRequestSize());
  }
  return RC_S_OK;
}

rc_t LogCollectorImpl::reqCollectorFile(bool_t bSend) {

  Request* pRequest = getRequest(0);
  if (NULL == pRequest) { return RC_S_NULL_VALUE; }

  // CollectorFile
  CollectorFileRequest* pCollectorFileRequest = pRequest->GetCollectorFileRequest();
  if (NULL == pCollectorFileRequest) { return RC_E_NOMEM; }

  if (TRUE == bSend) {
    if (RC_S_OK != pRequest->SerializeRequest()) { return RC_S_FAILED; }
    return m_autoRelINetHandler->Send(this, pRequest->GetRequestData(), pRequest->GetRequestSize());
  }
  return RC_S_OK;
}

rc_t LogCollectorImpl::reqCollectRule(bool_t bSend) {

  Request* pRequest = getRequest(0);
  if (NULL == pRequest) { return RC_S_NULL_VALUE; }

  // CollectorFile
  CollectRuleRequest* pCollectRuleRequest = pRequest->GetCollectRuleRequest();
  if (NULL == pCollectRuleRequest) { return RC_E_NOMEM; }

  if (TRUE == bSend) {
    if (RC_S_OK != pRequest->SerializeRequest()) { return RC_S_FAILED; }
    return m_autoRelINetHandler->Send(this, pRequest->GetRequestData(), pRequest->GetRequestSize());
  }
  return RC_S_OK;
}

rc_t LogCollectorImpl::reqFile(const char_t* strDir, const char_t* strName, uint64_t create_time) {

  //if (NULL == strDir || NULL == strName) { return RC_S_NULL_VALUE; }

  Request* pRequest = getRequest(0);
  if (NULL == pRequest) { return RC_S_NULL_VALUE; }

  // File
  FileRequest* pFileRequest = pRequest->GetFileRequest();
  if (NULL == pFileRequest) { return RC_E_NOMEM; }

  // send only
  if (NULL == strDir || NULL == strName) { 

    if (0 == pFileRequest->file_size() || RC_S_OK != pRequest->SerializeRequest()) { return RC_S_FAILED; }
    return m_autoRelINetHandler->Send(this, pRequest->GetRequestData(), pRequest->GetRequestSize());
  }

  const size_t encode_size = STRLEN(strDir) + STRLEN(strName) + 128;
  if (INetHandler::kMAX_REQ_DATA_SIZE <= pFileRequest->ByteSize() + encode_size) {
    if (RC_S_OK != pRequest->SerializeRequest()) {
      LOG(ILogWriter::LV_WARN, "File Packet EnCode Failed. len=%u(less)", pFileRequest->ByteSize() + encode_size);
      return RC_S_FAILED;
    }

    if (RC_S_OK != m_autoRelINetHandler->Send(this, pRequest->GetRequestData(), pRequest->GetRequestSize())) { return RC_S_FAILED; }
    pFileRequest = pRequest->GetFileRequest();
  }

  if (NULL == pFileRequest) { return RC_E_NOMEM; }
  FileRequest_File* pFile = pFileRequest->add_file();
  if (NULL == pFile) { return RC_E_NOMEM; }

  pFile->set_dir(strDir);
  pFile->set_name(strName);
  pFile->set_ctime(create_time);
  
  return RC_S_OK;
}

rc_t LogCollectorImpl::reqFileData(file_id_t file_id, file_size_t pos, uint8_t* data, uint32_t len) {

  Request* pRequest = getRequest(0);
  if (NULL == pRequest) { return RC_S_NULL_VALUE; }

  // FileData
  FileDataRequest* pFileDataRequest = pRequest->GetFileDataRequest();
  if (NULL == pFileDataRequest) { return RC_E_NOMEM; }

  // send only
  if (IFileNode::INVALID_FILE_ID == file_id || NULL == data || 0 == len) { 

    if (0 == pFileDataRequest->filedata_size() || RC_S_OK != pRequest->SerializeRequest()) { return RC_S_FAILED; }
    return m_autoRelINetHandler->Send(this, pRequest->GetRequestData(), pRequest->GetRequestSize());
  }

  //
  // deflate
  ASSERT(IFileCollector::kMAX_READ_SIZE >= len);
  size_t encode_size = m_autoRelMemDefalte->len();
  if (RC_S_OK != m_autoRelICoderDefalte->encode(m_autoRelMemDefalte->data(), &encode_size, data, len)) {
    LOG(ILogWriter::LV_WARN, "Deflate EnCode Failed, Org. Data Len=%u", len);
    return RC_S_FAILED;
  }

  // is too big
  if (INetHandler::kMAX_REQ_DATA_SIZE < encode_size) { return RC_E_ACCESS; }

  if (INetHandler::kMAX_REQ_DATA_SIZE < pFileDataRequest->ByteSize() + encode_size) {
    if (RC_S_OK != pRequest->SerializeRequest()) {
      LOG(ILogWriter::LV_WARN, "FileData Packet EnCode Failed. len=%u(less)", pFileDataRequest->ByteSize() + encode_size);
      return RC_S_FAILED;
    }

    if (RC_S_OK != m_autoRelINetHandler->Send(this, pRequest->GetRequestData(), pRequest->GetRequestSize())) { return RC_S_FAILED; }
    pFileDataRequest = pRequest->GetFileDataRequest();
  }

  // make sure have FileDataRequest
  if (NULL == pFileDataRequest) { return RC_E_NOMEM; }

  FileDataRequest_FileData* pFileData = pFileDataRequest->add_filedata();
  if (NULL == pFileData) { return RC_E_NOMEM; }

  FileID* pFileID = pFileData->mutable_id();
  if (NULL == pFileID) { return RC_E_NOMEM; }

  pFileID->set_id(file_id);
  pFileData->set_pos(pos);

  pFileData->set_encode(FileDataRequest_FileData_EncodeType_DEFLATE);
  pFileData->set_org_len(len);
  pFileData->set_data(m_autoRelMemDefalte->data(), encode_size);

  return RC_S_OK;
}

rc_t LogCollectorImpl::reqCheckSum(file_id_t file_id, uint32_t fb_cb_pos, uint32_t fb_cs_count) {

  Request* pRequest = getRequest(0);
  if (NULL == pRequest) { return RC_S_NULL_VALUE; }

  // checksum
  CheckSumRequest* pCheckSumRequest = pRequest->GetCheckSumRequest();
  if (NULL == pCheckSumRequest) { return RC_E_NOMEM; }

  FileID* pFileID = pCheckSumRequest->mutable_id();
  if (NULL == pFileID) { return RC_E_NOMEM; }
  pFileID->set_id(file_id);

  pCheckSumRequest->set_fb_cs_pos(fb_cb_pos);
  pCheckSumRequest->set_fb_cs_count(fb_cs_count);

  if (RC_S_OK != pRequest->SerializeRequest()) { return RC_S_FAILED; }
  return m_autoRelINetHandler->Send(this, pRequest->GetRequestData(), pRequest->GetRequestSize());
}

void LogCollectorImpl::checkSumFile(file_id_t) {

  // XXX
}

rc_t LogCollectorImpl::idle() {

  uint32_t now = DayTime::now();
  tickProc(now);

  if (TRUE == didLogon()) { collectFile(now); }
  return RC_S_OK;
}

rc_t LogCollectorImpl::collectFile(uint32_t now) {

  USING_BASE(DayTime);
  USING_BASE(Thread);

  // drop this
  if (m_queueSendFileNode.empty()) { Thread::sleep(m_nCollectorSleepTime); }

  static const uint32_t kCollectorMemMiniLogRuleNo = 0;
  // is time
  uint32_t nTimeRuleNO = kCollectorMemMiniLogRuleNo;
  uint32_t nStepTime = 0;
  uint32_t nKeepAlive = 60;
  uint32_t nSendRate = 0;
  uint32_t nWaitTime = m_CollectorConfig.didCollectFile(m_timeLastCollect
    , &nTimeRuleNO, &nStepTime, &nKeepAlive, &nSendRate);

  if (0 == nWaitTime 
    || (TRUE == m_bPackFileData && nWaitTime > (nStepTime + 1) / 2)
  ) {
    if (CollectorConfig::kDefaultSleepTime == nWaitTime && TRUE == m_bStrictMode) {
      // strict mode
      if (now - m_timeLastCollect > nKeepAlive) {
        LOG(ILogWriter::LV_TRACE, "Is happy time");
        m_timeLastCollect = now;
      }
    }
    else {

      m_bPackFileData = FALSE;
      ASSERT(m_autoRelINetHandler);
      if (nSendRate != m_nLastSendRate) {

        m_nLastSendRate = nSendRate;
        m_autoRelINetHandler->SetRateControl(nSendRate * 1024, FALSE);
      }

      {
        if (0 == nWaitTime) { LOG(ILogWriter::LV_TRACE, "Collect File"); }
        m_eCollectorType = kCollectorMemMiniLogRuleNo == nTimeRuleNO ? MEM_NINI_LOG : FULL_LOG;
        rc_t rc = fileCollect();
        if (RC_S_OK == rc) { m_timeLastCollectControl = 10; }
        m_timeLastCollect = now;

        static const uint32_t kKeepAliveControl = 3;
        if (kKeepAliveControl <= ++m_timeLastCollectControl) {

          m_timeLastCollectControl = 0;
          reqKeepAlive();
        }
      }
    }
    // Wait Aggregator Sync
  }
  else {

    if (CollectorConfig::kDefaultSleepTime == nWaitTime) {

      if (now - m_timeLastCollect > nKeepAlive) {
        LOG(ILogWriter::LV_TRACE, "Is happy time");
        m_timeLastCollect = now;
      }
    }
  }

  m_tcKeepAlive.SetTimeOut(nKeepAlive);
  m_autoRelINetHandler->SetKeepAlive(nKeepAlive);
  if (0 == m_tcKeepAlive.GetTimeOut() || m_tcKeepAlive.GetIdleTime(now)) { return RC_S_OK; }
  reqKeepAlive();

  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
rc_t LogCollectorImpl::initRequestAndResponse() {

  m_pRequestAndResponse = RequestAndResponse::CreateInstance();

  return m_pRequestAndResponse ? RC_S_OK : RC_E_NOMEM;
}

void LogCollectorImpl::releaseRequestAndResponse() {

  if (m_pRequestAndResponse) { m_pRequestAndResponse->Release(); }
}

//////////////////////////////////////////////////////////////////////////
END_NAMESPACE_COLLECTOR
