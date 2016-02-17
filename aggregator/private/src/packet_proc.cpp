/*

*/

#include "aggregator_impl.h"

#include "protocol.h"

#include "memory_pool.h"

#include "proto/aggregator.pb.h"
using namespace aggregator;

#include "google/protobuf/io/zero_copy_stream_impl_lite.h"

//////////////////////////////////////////////////////////////////////////
BEGIN_NAMESPACE_AGGREGATOR

USING_CODER(ICoder);
USING_BASE(MemoryNodeStd);

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


class RequestAndResponse {

public:
  void Release() { ASSERT(m_pPoolRequest); m_pPoolRequest->release(this); }

public:
  RequestAndResponse(AggregatorImpl* pool, uint32_t id)
    : m_nConnID(id)
    , m_pPoolRequest(pool)

    , m_AggregatorResponse()
    , m_pMemoryNodeStd(MemoryNodeStd::CreateInstance(AggregatorImpl::kDefaultPacketMemorySize))
    , m_bResponse(FALSE)
    , m_bSerialize(FALSE)

    , m_CollectorRequest()
    
    , m_pCollectClient(NULL)
    , m_pICoderDefalte(ICoder::CreateInstance(_STR("Deflate"), NULL))
  {}

  ~RequestAndResponse() { if (m_pMemoryNodeStd) { m_pMemoryNodeStd->Release(); } }

public:
  void SetConnID(uint32_t id) { m_nConnID = id; }

private:
  uint32_t                    m_nConnID;
  AggregatorImpl*             m_pPoolRequest;

  // response
public:
  inline AggregatorResponse& GetAggregatorResponse() { return m_AggregatorResponse; }

  inline LogonResponse* GetLogonResponse() { return m_AggregatorResponse.mutable_logon_response(); }

  inline KeepAliveResponse* GetKeepAliveResponse() { return m_AggregatorResponse.mutable_keepalive_response(); }
  inline LogResponse* GetLogResponse() { return m_AggregatorResponse.mutable_log_response(); }

  inline ConfigResponse* GetConfigResponse() { return m_AggregatorResponse.mutable_config_response(); }
  inline CheckSumResponse* GetCheckSumResponse() { return m_AggregatorResponse.mutable_checksum_response(); }

  inline CollectorFileResponse* GetCollectorFileResponse() { return m_AggregatorResponse.mutable_collectorfile_response(); }
  inline CollectRuleResponse* GetCollectRuleResponse() { return m_AggregatorResponse.mutable_collectrule_response(); }

  inline FileResponse* GetFileResponse() { return m_AggregatorResponse.mutable_file_response(); }
  inline FileDataResponse* GetFileDataResponse() { return m_AggregatorResponse.mutable_filedata_response(); }

  inline InstructResponse* GetInstructResponse() { return m_AggregatorResponse.mutable_instruct_response(); }

  IMemoryNode* GetResponseData() {
    if (FALSE == m_bResponse || NULL == m_pMemoryNodeStd) { return NULL; }
    m_pMemoryNodeStd->AddRef();
    return m_pMemoryNodeStd;
  }

  void ReSetResponse() {
    if (m_pMemoryNodeStd) { m_pMemoryNodeStd->SetSize(0); }
    m_bResponse = FALSE; m_bSerialize = FALSE;
  }

  rc_t SerializeResponse(uint32_t err_code = 0, const char_t* err_msg = NULL) {

    if (TRUE == m_bSerialize) { return RC_S_OK; }

    m_AggregatorResponse.set_error_code(err_code);
    if (err_msg) { m_AggregatorResponse.set_error_message(err_msg); }

    // trxid
    if (GetCollectorRequest().has_trxid_request()) {

      TransID* pTransID = m_AggregatorResponse.mutable_trxid_response();
      if (pTransID) { pTransID->set_id(GetCollectorRequest().trxid_request().id()); }
    }

    ASSERT(m_AggregatorResponse.IsInitialized());
    if (NULL == m_pMemoryNodeStd) { return RC_E_NOMEM; }
    google::protobuf::io::ArrayOutputStream zcos(m_pMemoryNodeStd->data(), AggregatorImpl::kDefaultPacketMemorySize);
    if (!m_AggregatorResponse.SerializeToZeroCopyStream(&zcos)) { return RC_S_FAILED; }

    m_pMemoryNodeStd->SetSize((uint32_t)(zcos.ByteCount()));
    m_AggregatorResponse.Clear();

    m_bSerialize = TRUE;
    return RC_S_OK;    
  }

  void SetResponse() { m_bResponse = TRUE; }
  bool_t hasResponse() const { return m_bResponse; }

private:
  AggregatorResponse          m_AggregatorResponse;
  MemoryNodeStd*              m_pMemoryNodeStd;
  bool_t                      m_bResponse;
  bool_t                      m_bSerialize;

  // request
public:
  inline const LogonRequest& GetLogonRequest() { return m_CollectorRequest.logon_request(); }
  inline const LogoutRequest& GetLogoutRequest() { return m_CollectorRequest.logout_request(); }
  inline const KeepAliveRequest& GetKeepAliveRequest() { return m_CollectorRequest.keepalive_request(); }
  inline const LogRequest& GetLogRequest() { return m_CollectorRequest.log_request(); }

  inline const ConfigRequest& GetConfigRequest() { return m_CollectorRequest.config_request(); }

  inline const CollectorFileRequest& GetCollectorFileRequest() { return m_CollectorRequest.collectorfile_request(); }
  inline const CollectRuleRequest& GetCollectRuleRequest() { return m_CollectorRequest.collectrule_request(); }

  inline const FileRequest& GetFileRequest() { return m_CollectorRequest.file_request(); }
  inline const FileDataRequest& GetFileDataRequest() { return m_CollectorRequest.filedata_request(); }
  inline const CheckSumRequest& GetCheckSumRequest() { return m_CollectorRequest.checksum_request(); }

  inline CollectorRequest& GetCollectorRequest() { return m_CollectorRequest; }

private:
  CollectorRequest            m_CollectorRequest;

public:
  void SetClient(AggregatorImpl::CollectClient* pCollectClient) { m_pCollectClient = pCollectClient; }
  AggregatorImpl::CollectClient* GetClient() { return m_pCollectClient; }

private:
  // client
  AggregatorImpl::CollectClient*              m_pCollectClient;

public:
  bool_t didCollectNow() { return NULL == m_pCollectClient ? FALSE : m_pCollectClient->didCollectNow(); }

public:
  ICoder* GetCoderDefalte() { return m_pICoderDefalte; }

private:
  ICoder*                     m_pICoderDefalte;

  DISALLOW_COPY_AND_ASSIGN(RequestAndResponse);
}; // RequestAndResponse

typedef RequestAndResponse    Request;
typedef RequestAndResponse    Response;

//////////////////////////////////////////////////////////////////////////
rc_t AggregatorImpl::packetProc(uint32_t nUniqueID, uint32_t len, const uint8_t* data) {

  CollectClient* pCollectClient = NULL;
  Request* pRequest = getRequestData(nUniqueID);
  if (NULL == pRequest) {
    m_autoRelIHostContext->Log(ILogWriter::LV_WARN, "Get Request Data Failed, %u", nUniqueID);
    return RC_S_NULL_VALUE; }

  pRequest->ReSetResponse();
  CollectorRequest& collectorRequest = pRequest->GetCollectorRequest();
  if (false == collectorRequest.ParseFromArray(data, len)) {
    m_autoRelIHostContext->Log(ILogWriter::LV_WARN, "Data Parser Failed, %u", nUniqueID);
    return RC_S_FAILED; }

  rc_t rc = RC_S_OK;
  if (collectorRequest.has_logon_request()) { 
    rc = packetProcLogon(pRequest);
    if (RC_S_OK != rc) {
      m_autoRelIHostContext->Log(ILogWriter::LV_WARN, "Logon Failed, %u", nUniqueID);
      return rc; }
    goto end;
  }  

  // check session
  pCollectClient = pRequest->GetClient();
  if (NULL == pCollectClient || !collectorRequest.has_session_id()) {
    m_autoRelIHostContext->Log(ILogWriter::LV_WARN, "Get Client Data Failed, %u", nUniqueID);
    return RC_S_NULL_VALUE; }

  pCollectClient->UpdateKeepAlive();

  if (pCollectClient->GetSessionID() != collectorRequest.session_id()) {
    m_autoRelIHostContext->Log(ILogWriter::LV_WARN, "Check session id failed. %s", pCollectClient->GetName());
    rc = RC_E_ACCESS; goto end;
  }

  if (collectorRequest.has_keepalive_request()) { rc = packetProcKeepAlive(pRequest); }
  if (RC_S_OK != rc) {
    m_autoRelIHostContext->Log(ILogWriter::LV_WARN, "packetProcKeepAlive Failed, %u", nUniqueID);
    goto end; }

  if (collectorRequest.has_log_request()) { rc = packetProcLog(pRequest); }
  if (RC_S_OK != rc) {
    m_autoRelIHostContext->Log(ILogWriter::LV_WARN, "packetProcLog Failed, %u", nUniqueID);
    goto end; }

  if (collectorRequest.has_config_request()) { rc = packetProcConfig(pRequest); }
  if (RC_S_OK != rc) {
    m_autoRelIHostContext->Log(ILogWriter::LV_WARN, "packetProcConfig Failed, %u", nUniqueID);
    goto end; }

  if (collectorRequest.has_collectorfile_request()) { rc = packetProcCollectorFile(pRequest); }
  if (RC_S_OK != rc) {
    m_autoRelIHostContext->Log(ILogWriter::LV_WARN, "packetProcCollectorFile Failed, %u", nUniqueID);
    goto end; }

  if (collectorRequest.has_collectrule_request()) { rc = packetProcCollectRule(pRequest); }
  if (RC_S_OK != rc) {
    m_autoRelIHostContext->Log(ILogWriter::LV_WARN, "packetProcCollectRule Failed, %u", nUniqueID);
    goto end; }

  if (collectorRequest.has_file_request()) { rc = packetProcFile(pRequest); }
  if (RC_S_OK != rc) {
    m_autoRelIHostContext->Log(ILogWriter::LV_WARN, "packetProcFile Failed, %u", nUniqueID);
    goto end; }

  if (collectorRequest.has_filedata_request()) { rc = packetProcFileData(pRequest); }
  if (RC_S_OK != rc) {
    m_autoRelIHostContext->Log(ILogWriter::LV_WARN, "packetProcFileData Failed, %u", nUniqueID);
    goto end; }

  if (collectorRequest.has_checksum_request()) { rc = packetProcCheckSum(pRequest); }
  if (RC_S_OK != rc) {
    m_autoRelIHostContext->Log(ILogWriter::LV_WARN, "packetProcCheckSum Failed, %u", nUniqueID);
    goto end; }

end:
  // RPC
  if (TRUE == pRequest->didCollectNow()) {

    InstructResponse* pInstructResponse = pRequest->GetInstructResponse();
    if (pInstructResponse) {

      uint32_t instruction = pInstructResponse->instruction() | InstructResponse_EncodeType_CollectNow;
      pInstructResponse->set_instruction(instruction);
      pRequest->SetResponse();
    }
  }

  // SerializeResponse
  if (TRUE == pRequest->hasResponse()) { pRequest->SerializeResponse(rc); }
  return rc;
}

rc_t AggregatorImpl::packetProcLogon(Request* pRequest) {

  ASSERT(pRequest);

  const LogonRequest& logonRequest = pRequest->GetLogonRequest();
  if (kProtocolVer != logonRequest.ver()) { return RC_S_CLOSED; }

  TRACE(logonRequest.name().c_str());

  rc_t rc;
  CollectClient* pCollectClient = checkLogon(rc, logonRequest.name().c_str(), TRUE);
  if (NULL == pCollectClient) { pRequest->SerializeResponse(rc); return RC_S_CLOSED; }

  // make Response
  LogonResponse* pLogonResponse = pRequest->GetLogonResponse();
  if (NULL == pLogonResponse) { return RC_E_NOMEM; }

  pLogonResponse->set_session_id(pCollectClient->GetSessionID());
  //pLogonResponse->set_ver(kProtocolVer);
  //pLogonResponse->set_name(m_strAggregator);

  if (RC_S_OK != pRequest->SerializeResponse()) { return RC_S_FAILED; }

  pRequest->SetClient(pCollectClient);
  pRequest->SetResponse();

  // log
  {
    const static char_t strLogonMsg[] = _STR("Logon");
    const static uint32_t nStrLogonMsgLen = sizeof(strLogonMsg) - 1;
    logDB(ILogWriter::LV_INFO, pCollectClient->GetName(), 
      (const uint8_t*)strLogonMsg, nStrLogonMsgLen, NULL, 0);
  }
  return RC_S_OK;
}

rc_t AggregatorImpl::packetProcLogout(Request* pRequest) {

  ASSERT(pRequest);
  CollectClient* pCollectClient = pRequest->GetClient();
  ASSERT(pCollectClient);

  pRequest->SetClient(NULL);

  // log
  const static char_t strLogoutMsg[] = _STR("Logout");
  const static uint32_t nStrLogoutMsgLen = sizeof(strLogoutMsg) - 1;

  // get msg
  const LogoutRequest& logoutRequest = pRequest->GetLogoutRequest();

  if (!logoutRequest.has_msg()) {
  
    logDB(ILogWriter::LV_INFO, pCollectClient->GetName(), 
      (const uint8_t*)strLogoutMsg, nStrLogoutMsgLen, NULL, 0);
    return RC_S_CLOSED;
  }

  logDB(ILogWriter::LV_INFO, pCollectClient->GetName(), 
    (const uint8_t*)strLogoutMsg, nStrLogoutMsgLen,
    (const uint8_t*)logoutRequest.msg().c_str(), (uint32_t)(logoutRequest.msg().length()));

  return RC_S_CLOSED;
}

rc_t AggregatorImpl::packetProcKeepAlive(Request* pRequest) {

  CollectClient* pCollectClient = pRequest->GetClient();
  ASSERT(pCollectClient);

  const KeepAliveRequest& keepAliveRequest = pRequest->GetKeepAliveRequest();
  pCollectClient->SetKeepAlive(keepAliveRequest.sync_pos());

  // make response
  KeepAliveResponse* pKeepAliveResponse = pRequest->GetKeepAliveResponse();
  if (NULL == pKeepAliveResponse) { return RC_S_FAILED; }

  pKeepAliveResponse->set_sync_pos(pCollectClient->GetRecvFileData());

  // FileDataResponse_FileData
  FileDataResponse* pFileDataResponse = pRequest->GetFileDataResponse();
  if (pCollectClient && pFileDataResponse) {

    pFileDataResponse->Clear();
    CollectClient::collecotor_file_list_t::const_iterator it_list, end;
    for (it_list = pCollectClient->m_listFileNode.begin(), end = pCollectClient->m_listFileNode.end();
      it_list != end; ++it_list) {

        AutoRelease<IFileNode*> autoRelIFileNode = GetFileNode((*it_list));
        if (NULL == autoRelIFileNode) { continue; }
        if (/*autoRelIFileNode->GetNFPos() < autoRelIFileNode->Size() || */IFileNode::FINISH == autoRelIFileNode->Status()) { continue; }

        FileDataResponse_FileData* pFileData = pFileDataResponse->add_filedata();
        if (NULL == pFileData) { continue; }

        FileID* pFileID = pFileData->mutable_id();
        if (NULL == pFileID) { continue; }

        pFileID->set_id(autoRelIFileNode->ID());
        pFileData->set_size(autoRelIFileNode->Size());
    }
  }

  pRequest->SetResponse();
  return RC_S_OK;
}

rc_t AggregatorImpl::packetProcLog(Request* pRequest) {

  ASSERT(pRequest);
  CollectClient* pCollectClient = pRequest->GetClient();
  ASSERT(pCollectClient);

  // get msg
  const LogRequest& logRequest = pRequest->GetLogRequest();

  if (!logRequest.has_content()) { return RC_S_CLOSED; }

  logDB(ILogWriter::LV_INFO, pCollectClient->GetName(), 
    (const uint8_t*)logRequest.data().c_str(), (uint32_t)(logRequest.data().length()),
    logRequest.has_content() ? (const uint8_t*)(logRequest.content().c_str()) : NULL,
    logRequest.has_content() ? (uint32_t)(logRequest.content().length()) : 0);

  return RC_S_OK;
}

rc_t AggregatorImpl::packetProcConfig(Request* pRequest) {

  CollectClient* pCollectClient = pRequest->GetClient();
  ASSERT(pCollectClient);

  ConfigResponse* pConfigResponse = pRequest->GetConfigResponse();
  if (NULL == pConfigResponse) { return RC_S_NULL_VALUE; }

  // config
  const CollectorConfig& collectorConfig = pCollectClient->GetCollectorConfig();
  const ConfigResponse* pConfigResponseClient = collectorConfig.GetConfigResponse();
  if (NULL == pConfigResponseClient) { return RC_S_NULL_VALUE; }

  pConfigResponse->CopyFrom(*pConfigResponseClient);
  pRequest->SetResponse();
  return RC_S_OK;
}

rc_t AggregatorImpl::packetProcCheckSum(Request* pRequest) {

  UNUSED_PARAM(pRequest);

  return RC_S_OK;
}

rc_t AggregatorImpl::packetProcCollectorFile(Request* pRequest) {

  ASSERT(pRequest);

  CollectClient* pCollectClient = pRequest->GetClient();
  ASSERT(pCollectClient);

  CollectorFileResponse* pCollectorFileResponse = pRequest->GetCollectorFileResponse();
  if (NULL == pCollectorFileResponse) { return RC_S_NULL_VALUE; }

  // config
  const CollectorConfig& collectorConfig = pCollectClient->GetCollectorConfig();
  const CollectorFileResponse* pCollectorFileResponseClient = collectorConfig.GetCollectorFileResponse();
  if (NULL == pCollectorFileResponseClient) { return RC_S_NULL_VALUE; }

  pCollectorFileResponse->CopyFrom(*pCollectorFileResponseClient);
  pRequest->SetResponse();
  return RC_S_OK;
}

rc_t AggregatorImpl::packetProcCollectRule(Request* pRequest) {

  ASSERT(pRequest);

  CollectClient* pCollectClient = pRequest->GetClient();
  ASSERT(pCollectClient);

  CollectRuleResponse* pCollectRuleResponse = pRequest->GetCollectRuleResponse();
  if (NULL == pCollectRuleResponse) { return RC_S_NULL_VALUE; }

  // config
  const CollectorConfig& collectorConfig = pCollectClient->GetCollectorConfig();

  const CollectRuleResponse* pCollectRuleResponseClient = collectorConfig.GetCollectRuleResponse();
  if (NULL == pCollectRuleResponseClient) { return RC_S_NULL_VALUE; }

  pCollectRuleResponse->CopyFrom(*pCollectRuleResponseClient);
  pRequest->SetResponse();
  return RC_S_OK;
}

rc_t AggregatorImpl::packetProcCollectNow(Request* pRequest) {

  UNUSED_PARAM(pRequest);


  return RC_S_OK;
}

rc_t AggregatorImpl::packetProcFile(Request* pRequest) {

  ASSERT(pRequest);

  CollectClient* pCollectClient = pRequest->GetClient();
  ASSERT(pCollectClient);

  const FileRequest& fileRequest = pRequest->GetFileRequest();
  
  //if (!fileRequest.has_dir() || !fileRequest.has_name()) { return RC_S_CLOSED; }

  FileResponse* pFileResponse = pRequest->GetFileResponse();
  if (NULL == pFileResponse) { return RC_S_NULL_VALUE; }

  for (int idx = 0; idx < fileRequest.file_size(); ++idx) {

    const FileRequest_File& file = fileRequest.file(idx);

    // make file id
    file_id_t file_id = IFileNode::INVALID_FILE_ID;
    /*rc_t rc = */makeFileID(file_id, pCollectClient->GetName(), file.dir().c_str(), file.name().c_str(), file.ctime());
    //if (RC_S_OK != rc) { continue; }

    pCollectClient->AddFile(file_id);

    // make response
    FileResponse_File* pFile = pFileResponse->add_file();
    if (NULL == pFile) { continue; }

    FileID* pFileID = pFile->mutable_id();
    if (NULL == pFileID) { continue; }

    pFileID->set_id(file_id);
    pFile->set_dir(file.dir());
    pFile->set_name(file.name());
  }

  if (0 >= pFileResponse->file_size()) { return RC_S_FAILED; }

  pRequest->SetResponse();
  return RC_S_OK;
}

rc_t AggregatorImpl::packetProcFileData(Request* pRequest) {

  ASSERT(pRequest);

  CollectClient* pCollectClient = pRequest->GetClient();
  ASSERT(pCollectClient);

  const FileDataRequest& fileDataRequest = pRequest->GetFileDataRequest();

  FileDataResponse* pFileDataResponse = pRequest->GetFileDataResponse();
  if (NULL == pFileDataResponse) { return RC_E_NOMEM; }

  for (int idx = 0; idx < fileDataRequest.filedata_size(); ++idx) {

    const FileDataRequest_FileData& fileData = fileDataRequest.filedata(idx);

    // update file data
    file_id_t file_id = fileData.id().id();
    EncodeType_e type = FileDataRequest_FileData_EncodeType_DEFLATE == fileData.encode() ? DEFLATE : NONE;
    const uint8_t* data = (const uint8_t *)(fileData.data().c_str());
    uint32_t dlen = (uint32_t)fileData.data().length();
    uint32_t olen = 0;
    if (fileData.has_org_len()) { olen = fileData.org_len(); }
    pCollectClient->SetRecvFileData(dlen, olen);
    rc_t rc = updateFileData(pRequest->GetCoderDefalte(), file_id, fileData.pos(), type, olen, dlen, data);
    if (RC_S_OK != rc) { return RC_S_CLOSED; }
  }

  // keepalive
  KeepAliveResponse* pKeepAliveResponse = pRequest->GetKeepAliveResponse();
  if (NULL == pKeepAliveResponse) { return RC_S_FAILED; }

  pKeepAliveResponse->set_sync_pos(pCollectClient->GetRecvFileData());

  pRequest->SetResponse();
  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
/*
rc_t AggregatorImpl::addRequestData(uint32_t nUniqueID) {

  AutoLock autoLock(m_lockRequestData);

  if (m_mapRequestData.end() == m_mapRequestData.find(nUniqueID)) { return RC_S_DUPLICATE; }

  Request* pRequest = createRequestData(nUniqueID);
  if (NULL == pRequest) { return RC_E_NOMEM; }

  m_mapRequestData[nUniqueID] = pRequest;
  return RC_S_OK;
}
*/
void AggregatorImpl::removeRequestData(uint32_t nUniqueID) {

  AutoLock autoLock(m_lockRequestData);

  packet_data_map_t::iterator it_map = m_mapRequestData.find(nUniqueID);
  if (m_mapRequestData.end() == it_map) { return; }

  // add to list
  if (it_map->second) { release_nolock(it_map->second); }

  m_mapRequestData.erase(nUniqueID);
}

Request* AggregatorImpl::getRequestData(uint32_t nUniqueID) {

  AutoLock autoLock(m_lockRequestData);

  packet_data_map_t::iterator it_map = m_mapRequestData.find(nUniqueID);
  if (m_mapRequestData.end() != it_map) { return it_map->second; }

  Request* pRequest = get_nolock();
  if (NULL == pRequest) { return NULL; }

  pRequest->SetConnID(nUniqueID);

  // add to map
  m_mapRequestData[nUniqueID] = pRequest;
  return pRequest;
}

Request* AggregatorImpl::createRequestData(uint32_t nUniqueID) {

  Request* pRequest = NULL;
  if (RC_S_OK == get(&pRequest)) {

    ASSERT(pRequest);
    pRequest->SetConnID(nUniqueID);
  }
  return pRequest;
}

Request* AggregatorImpl::get_nolock() {

  while (m_listRequest.size()) {

    Request* pRequest = m_listRequest.front();
    m_listRequest.pop_front();

    if (pRequest) { return pRequest; }
  }

  return new Request(this, 0);
}

void AggregatorImpl::release_nolock(Request* pRequest) {

  m_listRequest.push_back(pRequest);
}


rc_t AggregatorImpl::get(Request** ppRequest, uint32_t /*flag*/) {

  if (NULL == ppRequest) { return RC_S_NULL_VALUE; }

  Request* pRequest = NULL;
  {
    AutoLock autoLock(m_lockRequestData);
    while (m_listRequest.size()) {

      pRequest = m_listRequest.front();
      m_listRequest.pop_front();

      if (pRequest) {
        (*ppRequest) = pRequest;
        return RC_S_OK;
      }
    }
  }

  pRequest = new Request(this, 0);
  return pRequest ? RC_S_OK : RC_E_NOMEM;
}

void AggregatorImpl::release(Request* pRequest) {

  AutoLock autoLock(m_lockRequestData);

  release_nolock(pRequest);
}

rc_t AggregatorImpl::initRequestData() {

  return RC_S_OK;
}

void AggregatorImpl::clearRequestData() {

  AutoLock autoLock(m_lockRequestData);

  Request_list_t::iterator it_list, end;
  for (it_list = m_listRequest.begin(), end = m_listRequest.end(); it_list != end; ++it_list) {

    if (NULL == *it_list) { continue; }
    delete (*it_list);
  }
  m_listRequest.clear();
}

IMemoryNode* AggregatorImpl::getResponsePacketData(uint32_t nUniqueID) {

  Request* pRequest = getRequestData(nUniqueID);
  if (NULL == pRequest) { return NULL; }

  return pRequest->GetResponseData();
}

//////////////////////////////////////////////////////////////////////////
END_NAMESPACE_AGGREGATOR

