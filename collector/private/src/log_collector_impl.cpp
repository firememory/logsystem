/*

*/

#include "log_collector_impl.h"

#include "file_collector_impl.h"

#include "xml.h"
#include "net_util.h"
#include "code_util.h"

#include "thread.h"

#ifdef _MSC_VER
# pragma warning(disable: 4127)
#endif // _MSC_VER


//////////////////////////////////////////////////////////////////////////
BEGIN_NAMESPACE_COLLECTOR
//////////////////////////////////////////////////////////////////////////
USING_BASE(AutoLock);
USING_BASE(ToBase16_WT);
USING_BASE(OpenFile);
USING_BASE(WriteFile);
USING_BASE(strAttrOpenWrite);

USING_NET(IPAddressToString);

//////////////////////////////////////////////////////////////////////////

static const uint32_t kWaitSendTimeOut  = 15 * 60;
//////////////////////////////////////////////////////////////////////////
LogCollectorImpl::LogCollectorImpl(IHostContext* pIHostContext, INetHandler* pINetHandler)
  : m_eStatus(RC_S_UNKNOWN)
  , m_autoRelIHostContext(pIHostContext)
  , m_autoRelINetHandler(pINetHandler)
  , m_autoRelIFileCollector(new FileCollectorImpl())
  , m_pRequestAndResponse(NULL)
  , m_bConfigName(FALSE)
  , m_CollectorConfig(m_strCollector)
  , m_timeLastCollect(0)
  , m_timeLastCollectControl(0)
  , m_autoRelICoderDefalte(ICoder::CreateInstance(_STR("Deflate"), NULL))
  , m_autoRelMemRead(MemoryNodeStd::CreateInstance(IFileCollector::kMAX_READ_SIZE))
  , m_autoRelMemDefalte(MemoryNodeStd::CreateInstance(IFileCollector::kMAX_DEFALT_BUF_SIZE))
  , m_bStop(FALSE)
  , m_nLastSendRate(0)
  , m_nLastKeepAlive(0)
  , m_tcKeepAlive(60)

  , m_nSyncPos(0)
  , m_nAggSyncPos(0)

  , m_bSleep(FALSE)
  , m_nSleepedTime(0)

#if defined(DEBUG_ON)
  , m_nConnectWaitTime(2 * 1000) // 2s
#else
  , m_nConnectWaitTime(3 * 60 * 1000) // 3 * 60s
#endif //

  , m_bPackFileData(FALSE)

  , m_tcTick(kWaitSendTimeOut)

  , m_nCollectorSleepTime(300)
  , m_bStrictMode(TRUE)

  , m_listMemMiniLogIncludeLog()
  , m_eCollectorType(FULL_LOG)
{
  BZERO_ARR(m_strAggregator);
  initRequestAndResponse();
  LOG(ILogWriter::LV_INFO, "%s", "Collector " STR_VERSION);
}

LogCollectorImpl::~LogCollectorImpl() {

  clearQueueSendFileNode();
  clearQueueReqFileNode();

  //clearMapFileNode();

  releaseRequestAndResponse();
}

//////////////////////////////////////////////////////////////////////////
// xml config
const char_t* strCFGFile                = _STR("collector.xml");
const char_t* strCollector              = _STR("collector");

const char_t* strSystem                 = _STR("System");
const char_t* strAttrConnectWaitTime    = _STR("ConnectWaitTime");

const char_t* strTimeControl            = _STR("TimeControl");
const char_t* strAttrAfterToday         = _STR("AfterToday");
const char_t* strAttrSleepTime          = _STR("SleepTime");
const char_t* strAttrStrictMode         = _STR("StrictMode");

const char_t* strAttrIncludeLog         = _STR("IncludeLog");

const char_t* strAttrName               = _STR("name");
const char_t* strAttrPath               = _STR("path");
const char_t* strAttrEnable             = _STR("enable");
const char_t* strValYes                 = _STR("yes");

const char_t* strLog                    = _STR("log");
const char_t* strAttrLevel              = _STR("level");
const char_t* strAttrPoolSize           = _STR("pool_size");

const char_t* strFileSystem             = _STR("filesystem");
const char_t* strAttrCacheCount         = _STR("cache_count");
const char_t* strAttrCCFileBlock        = _STR("cache_count_file_block");

const char_t* strDataBase               = _STR("database");
const char_t* strDBServer               = _STR("server");
//////////////////////////////////////////////////////////////////////////

void LogCollectorImpl::expatStart(void *data, const XML_Char *el, const XML_Char **attr) {

  if (NULL == data || NULL == el || NULL == attr) { return; }
  
  LogCollectorImpl* pLogCollectorImpl = (LogCollectorImpl*)(data);
  ASSERT(pLogCollectorImpl);


  // m_strAggregator
  if (0 == STRICMP(strCollector, el)) {

    if (NULL == attr[0] || 0 != STRICMP(strAttrName, attr[0]) || NULL == attr[1]) { return; }
    const char_t* strVal = attr[1];

    ASSERT(pLogCollectorImpl->m_autoRelIHostContext);

    pLogCollectorImpl->setCollector(strVal);
    pLogCollectorImpl->m_bConfigName = TRUE;
  }

  if (0 == STRICMP(strSystem, el)) {

    if (attr[0] && 0 == STRICMP(strAttrConnectWaitTime, attr[0]) && attr[1]) {
      const char_t* strVal = attr[1];
      uint32_t nConnectWaitTime = ATOI(strVal);
      if (0 == nConnectWaitTime) { nConnectWaitTime = 180; }
      pLogCollectorImpl->m_nConnectWaitTime = nConnectWaitTime * 1000;
      pLogCollectorImpl->m_autoRelIHostContext->Log(ILogWriter::LV_TRACE, "ConnectWaitTime Is %u(s)", nConnectWaitTime);
    }
  }

  if (0 == STRICMP(strTimeControl, el)) {

    // aftertoday
    if (attr[0] && 0 == STRICMP(strAttrAfterToday, attr[0]) && attr[1]) {
      const char_t* strVal = attr[1];

      ASSERT(pLogCollectorImpl->m_autoRelIHostContext);
      ASSERT(pLogCollectorImpl->m_autoRelIFileCollector);

      if ('Y' == *strVal) {
        pLogCollectorImpl->m_autoRelIFileCollector->SetTimeControl(TRUE);
        pLogCollectorImpl->m_autoRelIHostContext->Log(ILogWriter::LV_TRACE, "TimeControl Is Enable");
      }
      else {
        pLogCollectorImpl->m_autoRelIFileCollector->SetTimeControl(FALSE);
        pLogCollectorImpl->m_autoRelIHostContext->Log(ILogWriter::LV_TRACE, "TimeControl Is Disable");
      }
    }

    // sleep time
    if (attr[2] && 0 == STRICMP(strAttrSleepTime, attr[2]) && attr[3]) {
      pLogCollectorImpl->m_nCollectorSleepTime = ATOI(attr[3]);
      pLogCollectorImpl->m_autoRelIHostContext->Log(ILogWriter::LV_TRACE, "SleepTime Is %u(ms)", pLogCollectorImpl->m_nCollectorSleepTime);
    }

    // StrictMode
    if (attr[4] && 0 == STRICMP(strAttrStrictMode, attr[4]) && attr[5]) {
      const char_t* strVal = attr[5];

      ASSERT(pLogCollectorImpl->m_autoRelIHostContext);

      if ('Y' == *strVal) {
        pLogCollectorImpl->m_bStrictMode = TRUE;
        pLogCollectorImpl->m_autoRelIHostContext->Log(ILogWriter::LV_TRACE, "TimeControl StrictMode Is Enable");
      }
      else {
        pLogCollectorImpl->m_bStrictMode = FALSE;
        pLogCollectorImpl->m_autoRelIHostContext->Log(ILogWriter::LV_TRACE, "TimeControl StrictMode Is Disable");
      }
    }
  }

  if (0 == STRICMP(strMemMiniLog, el)) {

    if (attr[0] && 0 == STRICMP(strAttrIncludeLog, attr[0]) && attr[1]) {
      const char_t* strVal = attr[1];
      pLogCollectorImpl->setMemMiniLogIncludeLog(strVal);
      pLogCollectorImpl->m_autoRelIHostContext->Log(ILogWriter::LV_TRACE, "MemMiniLog IncludeLog=%s", strVal);
    }
  }
}

void LogCollectorImpl::expatEnd(void *data, const XML_Char *el) {
  if (NULL == data || NULL == el) { return; }
}

rc_t LogCollectorImpl::defaultConfig() {

  // name
  size_t nIPLen = 0;
  size_t nMACLen = 0;
  const uint8_t* pIP = m_autoRelINetHandler->GetLocalIP(&nIPLen);
  const uint8_t* pMAC = m_autoRelINetHandler->GetLocalMAC(&nMACLen);

  if (pIP && pMAC && nIPLen && nMACLen) {

    char_t strCollectorName[kMAX_NAME_LEN + 1] = {0};
    size_t len = IPAddressToString(strCollectorName, pIP, nIPLen);

    if (len) {
      // ipv6
      const size_t kMAX_IP_LEN    = 16;
      if (len > kMAX_IP_LEN) {
        MEMMOVE(strCollectorName, sizeof(strCollectorName), strCollectorName + (len - kMAX_IP_LEN), kMAX_IP_LEN);
        len = kMAX_IP_LEN;
      }

      strCollectorName[len] = _CHAR('_');
      len += sizeof(char_t);
      ToBase16_WT((uint8_t*)strCollectorName + len, pMAC, nMACLen);
      len += 2 * nMACLen;
      strCollectorName[len] = 0x00;
      setCollector(strCollectorName);
    }
  }
  else { return RC_S_FAILED; }

  //
  m_strLastEL = NULL;

  return RC_S_OK;
}

rc_t LogCollectorImpl::readConfig() {

  // read Configure
  char_t strPath[kMAX_PATH + 1] = {0};

  SNPRINTF(strPath, kMAX_PATH, _STR("%s/%s"), m_autoRelIHostContext->GetSystemExeFullPath(), strCFGFile);

  AutoReleaseFile AutoRelFile(OpenFile(strPath, strAttrOpenRead));
  if (NULL == AutoRelFile) {

    if (RC_S_OK != defaultConfig()) {
      LOG(ILogWriter::LV_INFO, "Default Config Failed", strPath);
      return RC_S_FAILED;
    }

    LOG(ILogWriter::LV_INFO, "Not Found Config File, Will Make Default Config File", strPath);

    AutoRelFile.Set(OpenFile(strPath, strAttrOpenWrite));
    if (NULL == AutoRelFile) { return RC_S_NOTFOUND; }

    char_t strXML[1024] = {0};
    size_t len = SNPRINTF(strXML, sizeof(strXML), sizeof(strXML),
      "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
      "<Collector name=\"%s\" >\r\n"
      "  <System ConnectWaitTime=\"180\" />\r\n"
      "  <TimeControl AfterToday=\"Y\" SleepTime=\"300\" StrictMode=\"Y\" />\r\n"
      "  <MemMiniLog IncludeLog=\"(0-94),(0-96),(0-98),(0-100)\" />\r\n"
      "</Collector>\r\n",
      m_strCollector
    );

    file_size_t writeSize = len;
    if (RC_S_OK != WriteFile(AutoRelFile, 0, (const uint8_t*)(strXML), &writeSize)) { return RC_S_FAILED; }

    AutoRelFile.Set(OpenFile(strPath, strAttrOpenRead));
    if (NULL == AutoRelFile) { return RC_S_FAILED; }
  }

  PlatformFileInfo file_info;
  if (FALSE == GetPlatformFileInfo(AutoRelFile, &file_info) || 0 == file_info.size) { return RC_S_NOTFOUND; }

  //file_size_t xmlSize = file_info.size;
  const uint32_t kMAX_CONFIG_FILE_DATA_LEN = 4* 1024;
  uint8_t FileData[kMAX_CONFIG_FILE_DATA_LEN] = {0};

  if (file_info.size > kMAX_CONFIG_FILE_DATA_LEN) {

    LOG(ILogWriter::LV_INFO, "Config File Only Read %u Bytes", kMAX_CONFIG_FILE_DATA_LEN);
  }

  file_size_t xmlSize = kMAX_CONFIG_FILE_DATA_LEN;
  if (RC_S_OK != ReadFile(FileData, &xmlSize, AutoRelFile, 0)) { return RC_S_NOTFOUND; }

  AutoReleaseXML autoRelXML(XML_ParserCreate(NULL));
  if (NULL == autoRelXML) { return RC_E_NOMEM; }

  XML_SetUserData(autoRelXML, this);
  XML_SetElementHandler(autoRelXML, expatStart, expatEnd);
  if (XML_STATUS_ERROR == 
    XML_Parse(autoRelXML, (const char*)FileData, (int)xmlSize, 1)) { return RC_S_FAILED; }
  return RC_S_OK;
}

rc_t LogCollectorImpl::init() {

  if (NULL == m_autoRelICoderDefalte || NULL == m_autoRelMemRead || NULL == m_autoRelMemDefalte) { return RC_S_FAILED; }

  if (RC_S_OK != readConfig()) {
    LOG(ILogWriter::LV_INFO, "Parser Config Failed");
    return RC_S_FAILED;
  }

  LOG(ILogWriter::LV_TRACE, "%s, INIT OK", m_strCollector);
  return RC_S_OK;
}

rc_t LogCollectorImpl::Start() {

  rc_t rc;
  ASSERT(m_autoRelINetHandler);

  if (FALSE == didRequestAndResponse()) {

    LOG(ILogWriter::LV_ERR, "Alloc Request And Response Data Failed");
    return RC_E_NOMEM;
  }

  rc = init();
  if (RC_S_OK != rc) {
    LOG(ILogWriter::LV_ERR, "Init Failed");
    return rc;
  }

  m_autoRelINetHandler->SetNetEvent(this);

  //m_autoRelINetHandler->Set(NetHandlerImpl::m_strAttrUser, _STR("collector"), STRLEN(_STR("collector")));
  //m_autoRelINetHandler->Set(NetHandlerImpl::m_strAttrPWD, _STR("collector"));

  if (RC_S_OK != (rc = m_autoRelINetHandler->InitNet())
    || RC_S_OK != (rc = m_autoRelINetHandler->StartNet())
  ) {

    LOG(ILogWriter::LV_ERR, "Taapi Engine Start Failed");
    return rc;
  }

  m_eStatus = RC_S_INIT;
  m_bStop = FALSE;

  LOG(ILogWriter::LV_TRACE, "Start OK");
  return RC_S_OK;
}

rc_t LogCollectorImpl::Stop() {

  rc_t rc;
  ASSERT(m_autoRelINetHandler);

  // close 
  m_bStop = TRUE;
  m_eStatus = RC_S_CLOSED; Run();

  if (RC_S_OK != (rc = m_autoRelINetHandler->StopNet())
    || RC_S_OK != (rc = m_autoRelINetHandler->DeInitNet())
  ) {

    LOG(ILogWriter::LV_ERR, "Taapi Engine Stop Failed");
    return rc;
  }

  LOG(ILogWriter::LV_TRACE, "Stop OK");
  return RC_S_OK;
}

rc_t LogCollectorImpl::ReStart() {

  rc_t rc;
  ASSERT(m_autoRelINetHandler);

  // close 
  m_eStatus = RC_S_CLOSED; Run();

  if (RC_S_OK != (rc = m_autoRelINetHandler->StopNet())
    || RC_S_OK != (rc = m_autoRelINetHandler->StartNet())
  ) {

    LOG(ILogWriter::LV_ERR, "Taapi Engine Stop Failed");
    return rc;
  }

  LOG(ILogWriter::LV_TRACE, "ReStart OK");
  m_eStatus = RC_S_INIT;
  return RC_S_OK;
}

void LogCollectorImpl::tickProc(uint32_t now) {

  if (m_tcTick.GetIdleTime(now)) { return; }

  if (NULL == m_autoRelIFileCollector) { return; }

  m_autoRelIFileCollector->TickProc();
}

USING_BASE(Thread);

const uint32_t kPreWaitTime             = 300; //

rc_t LogCollectorImpl::Run() {

  rc_t rc;
  ASSERT(m_autoRelINetHandler);
  if (TRUE == m_bSleep) {

    if (m_nConnectWaitTime > m_nSleepedTime) {
      Thread::sleep(kPreWaitTime);
      m_nSleepedTime += kPreWaitTime;
      return RC_S_OK;
    }
    else { m_nSleepedTime = 0; m_bSleep = FALSE; }
  }

  //AutoLock autoLock(m_lockLogCollector);

  switch(m_eStatus) {
  case RC_S_INIT: {
    // open
    rc = m_autoRelINetHandler->Open();
    if (RC_S_OK == rc) {
      m_eStatus = RC_S_OPEN;
    }
    else {
      rc = m_autoRelINetHandler->Close();
      LOG(ILogWriter::LV_ERR, "NetWork Failed. Wait %u Sec", m_nConnectWaitTime / 1000);
      m_bSleep = TRUE;
    }
    break;
  }
  case RC_S_OPEN: {
    // login
    rc = reqLogon();
    if (RC_S_OK == rc) {
      LOG(ILogWriter::LV_INFO, "Connect OK");
      m_eStatus = RC_S_RUNNING;
    }
    else {
      LOG(ILogWriter::LV_ERR, "Connect Failed. Wait %u Sec", m_nConnectWaitTime / 1000);
      m_bSleep = TRUE;
    }
    break;
  }
  case RC_S_RUNNING: {
    // all config
    rc = reqAllConfig();
    if (RC_S_OK == rc) {
      LOG(ILogWriter::LV_INFO, "Request Config OK");
      m_eStatus = RC_S_IDLE;
    }
    else {
      LOG(ILogWriter::LV_ERR, "NetWork Failed. Wait %u Sec", m_nConnectWaitTime / 1000);
      m_bSleep = TRUE;
    }
    break;
  }
  case RC_S_IDLE: {
    // work
    rc = idle();
    if (RC_S_OK != rc) {
      m_eStatus = RC_S_CLOSED;
    }
    break;
  }
  case RC_S_CLOSED: {
    // close
    setLogon(FALSE);
    rc = m_autoRelINetHandler->Close();
    if (TRUE == m_bStop) { break; }
    m_bSleep = TRUE;
    m_eStatus = RC_S_INIT;

    // 
    clearFileNode();
    break;
  }
  case RC_S_UNKNOWN: break;
  default:
    break;
  }

  return RC_S_OK;
}

rc_t LogCollectorImpl::NetEvent(const void* context, net_event_e type, uint32_t len, const uint8_t* data) {
  
  UNUSED_PARAM(context);
  UNUSED_PARAM(len);
  UNUSED_PARAM(data);

  if (DISCONNECT == type) {
    m_eStatus = RC_S_CLOSED;
    LOG(ILogWriter::LV_INFO, "DisConnect. Wait %u Sec", m_nConnectWaitTime / 1000);
  }

  if (RESPONSE_DATA == type) {

    const uint32_t kConnID = 1;
    if (RC_S_CLOSED == packetProc(kConnID, len, data)) {

      LOG(ILogWriter::LV_ERR, "Packet Proc Failed. Will Close The Connection", m_nConnectWaitTime / 1000);
      m_eStatus = RC_S_CLOSED;
    }
  }

  return RC_S_OK;
}

rc_t LogCollectorImpl::slowDown() {

  m_autoRelINetHandler->SetRateControl(m_nLastSendRate, TRUE);
  return RC_S_OK;
}

rc_t LogCollectorImpl::sendRequestFileNode() {

  /*
  // send req
  while (m_queueReqFileNode.empty()) {

    const IFileNode* pIFileNode = m_queueReqFileNode.front();
    m_queueReqFileNode.pop();

    UNUSED_LOCAL_VARIABLE(pIFileNode);
  }

  // send file
  while (m_queueSendFileNode.empty()) {

    FileNode* pFileNode = m_queueSendFileNode.front();
    m_queueSendFileNode.pop();

    UNUSED_LOCAL_VARIABLE(pFileNode);
  }
  */
  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
rc_t LogCollectorImpl::addSendQueue(file_id_t file_id) {

  IFileNode* pIFileNode = findFileNode(file_id);
  if (NULL == pIFileNode) { return RC_S_NOTFOUND; }

  m_queueSendFileNode.push(pIFileNode);
  return RC_S_OK;
}

rc_t LogCollectorImpl::addReqQueue(const IFileNode* pFileNode) {

  if (NULL == pFileNode) { return RC_S_NULL_VALUE; }

  m_queueReqFileNode.push(pFileNode);
  return RC_S_OK;
}
/*
rc_t LogCollectorImpl::addMapFileNode(file_id_t file_id, const char_t* strDir, const char_t* strName) {

  if (NULL == m_autoRelIFileCollector) { return RC_S_NULL_VALUE; }

  if (findFileNode(file_id)) { return RC_S_DUPLICATE; }

  IFileNode* pIFileNode = findFileNode(strDir, strName);
  if (NULL == pIFileNode) { return RC_S_NOTFOUND; }

  FileNode* pFileNode = new FileNode(file_id, pIFileNode);
  if (NULL == pFileNode) { return RC_E_NOMEM; }

  m_mapFileNode[file_id] = pFileNode;
  pIFileNode->SetID(file_id);
  return RC_S_OK;
}

rc_t LogCollectorImpl::updateMapFileNode(file_id_t file_id, file_size_t file_pos) {

  if (NULL == m_autoRelIFileCollector) { return RC_S_NULL_VALUE; }

  FileNode* pFileNode = findFileNode(file_id);
  if (NULL == pFileNode) { return RC_S_NOTFOUND; }

  IFileNode* pIFileNode = pFileNode->GetIFileNode();
  if (NULL == pIFileNode) { return RC_S_NOTFOUND; }

  pIFileNode->SetSize(file_pos);

  return RC_S_OK;
}

void LogCollectorImpl::clearMapFileNode() {

  file_node_map_t::iterator it_map, end;
  for (it_map = m_mapFileNode.begin(), end = m_mapFileNode.end(); it_map != end; ++it_map) {

    FileNode* pFileNode = it_map->second;
    if (NULL == pFileNode) { continue; }

    delete pFileNode;
  }

  m_mapFileNode.clear();
}
*/
void LogCollectorImpl::clearQueueSendFileNode() {

  while (!m_queueSendFileNode.empty()) {
    m_queueSendFileNode.pop();
  }
}

void LogCollectorImpl::clearQueueReqFileNode() {

  while (!m_queueReqFileNode.empty()) {
    m_queueReqFileNode.pop();
  }
}

//////////////////////////////////////////////////////////////////////////
rc_t LogCollectorImpl::addCollectRule(const char_t* strDir, const char_t* strExclude, const char_t* strInclude) {

  ASSERT(strDir && strExclude && strInclude);
  if (NULL == m_autoRelIFileCollector) { return RC_S_NULL_VALUE; }

  LOG(ILogWriter::LV_TRACE, "Collect Rule: Dir[%s], Exclude[%s], Inlcude[%s]", strDir, strExclude, strInclude);
  return m_autoRelIFileCollector->AddCollectRule(strDir, strExclude, strInclude);
}

void LogCollectorImpl::clearCollectRule() {

  if (NULL == m_autoRelIFileCollector) { return; }

  m_autoRelIFileCollector->ClearCollectRule();
}

IFileNode* LogCollectorImpl::addFileNode(const file_id_t& file_id, file_size_t pos, bool_t bFinish, const char_t* strDir, const char_t* strName) {

  if (NULL == m_autoRelIFileCollector) { return NULL; }

  LOG(ILogWriter::LV_TRACE, "File: ID[%u], Dir[%s], Name[%s], size=%I64u", file_id, strDir, strName, pos);
  
  return m_autoRelIFileCollector->AddFileNode(file_id, pos, bFinish, strDir, strName);
}

rc_t LogCollectorImpl::updateFileNode(const file_id_t& file_id, file_size_t size) {

  IFileNode* pIFileNode = m_autoRelIFileCollector->FindFileNode(file_id);
  if (NULL == pIFileNode) { return RC_S_NOTFOUND; }

  pIFileNode->SetSize(size);
  return RC_S_OK;
}

IFileNode* LogCollectorImpl::findFileNode(const char_t* strDir, const char_t* strName) {

  if (NULL == m_autoRelIFileCollector) { return NULL; }

  return m_autoRelIFileCollector->FindFileNode(strDir, strName);
}

IFileNode* LogCollectorImpl::findFileNode(const file_id_t& file_id) {

  if (NULL == m_autoRelIFileCollector) { return NULL; }

  return m_autoRelIFileCollector->FindFileNode(file_id);

}

void LogCollectorImpl::clearFileNode() {

  if (NULL == m_autoRelIFileCollector) { return; }

  clearQueueReqFileNode();
  clearQueueSendFileNode();
  m_autoRelIFileCollector->ClearFileNode();
}

rc_t LogCollectorImpl::fileCollectCallback(/*IFileCollector* pIFileCollector,*/ const IFileNode* pFileNode, void* context) {

  if (/*NULL == pIFileCollector || */NULL == context) { return RC_E_NOMEM; }

  LogCollectorImpl* pLogCollectorImpl = (LogCollectorImpl*)(context);
  ASSERT(pLogCollectorImpl);

  // add request queue
  if (IFileNode::INVALID_FILE_ID == pFileNode->ID()) {
    pLogCollectorImpl->addReqQueue(pFileNode);
    return RC_S_OK;
  }

  //
  pLogCollectorImpl->addSendQueue(pFileNode->ID());
  return RC_S_OK;  
}

rc_t LogCollectorImpl::fileCollect() {

  m_autoRelIFileCollector->Collector(m_eCollectorType, fileCollectCallback, this);

  return procFileQueue();
}

rc_t LogCollectorImpl::procFileQueue() {

  // new file
  bool_t bSendLast = m_queueReqFileNode.empty() ? FALSE : TRUE;
  while (!m_queueReqFileNode.empty()) {

    const IFileNode* pIFileNode = m_queueReqFileNode.front();
    m_queueReqFileNode.pop();

    if (NULL == pIFileNode) { continue; }

    FileCollectorImpl::FileNode* pFileNodeFC = (FileCollectorImpl::FileNode*)pIFileNode;
    if (NULL == pFileNodeFC) { continue; }
    
    if (MEM_NINI_LOG == m_eCollectorType && pFileNodeFC->GetMMLDir()) {
      reqFile(pFileNodeFC->GetMMLDir(), pFileNodeFC->LocalName(), pFileNodeFC->CreateTime());
    }
    else {
      reqFile(pFileNodeFC->LocalDir(), pFileNodeFC->LocalName(), pFileNodeFC->CreateTime());
    }

    LOG(ILogWriter::LV_TRACE, "New File. [%s]", pFileNodeFC->LocalPath());
  }

  // send all req file id
  if (bSendLast) { reqFile(NULL, NULL, 0); }

  // file data
  bSendLast = FALSE;
  //bSendLast = m_queueSendFileNode.empty() ? FALSE : TRUE;
  while (!m_queueSendFileNode.empty()) {

    IFileNode* pIFileNode = m_queueSendFileNode.front();
    m_queueSendFileNode.pop();

    FileCollectorImpl::FileNode* pFileNodeFC = (FileCollectorImpl::FileNode*)pIFileNode;
    if (NULL == pFileNodeFC || FALSE == pFileNodeFC->DidSend()) { continue; }

    if (MEM_NINI_LOG != m_eCollectorType && pFileNodeFC->GetMMLDir()) { continue; }

    file_id_t file_id = pFileNodeFC->ID();
    file_size_t nPrevSendPos = pFileNodeFC->GetSendSize();
    file_size_t nReadLen = 0;
    do {

      // get file data
      file_size_t nReadPos = pFileNodeFC->GetSendSize();
      nReadLen = pFileNodeFC->GetMaxEncodeSize();

      ASSERT(m_autoRelMemRead->len() >= nReadLen);

      // read over
      //if (nReadPos >= pFileNodeFC->Size()) { break; }
      if (RC_S_OK != pFileNodeFC->GetData(m_autoRelMemRead->data(), &nReadLen, nReadPos)) {

        LOG(ILogWriter::LV_WARN, "Read File Failed. Path[%s], Pos=%I64u, Len=%u"
          , pFileNodeFC->LocalPath(), nReadPos, pFileNodeFC->GetMaxEncodeSize());
        break;
      }

      ASSERT(nReadLen < (uint64_t)kuint32max);
      preProcLogFile(&(pFileNodeFC->m_nMemMiniLogData), m_autoRelMemRead->data(), (uint32_t)nReadLen);

      //const uint32_t kPRE_DEC_SIZE = 4 * 1024;
      rc_t rc;
      uint32_t nTryCount = 0;
      // is too big
      while (RC_E_ACCESS == (rc = reqFileData(file_id, nReadPos, m_autoRelMemRead->data(), (uint32_t)nReadLen))) {

        // dec data len;
        if (nReadLen < 32 * 1024) { rc = RC_S_FAILED; break; }
        nReadLen /= 2;
        ++nTryCount;
      }

      // can not process this error
      if (RC_S_OK != rc) { break; }
      if (FALSE == bSendLast) { bSendLast = TRUE; }
      // update file code size
      //else { pFileNodeFC->SetMaxEncodeSize(0 == nTryCount ? (uint32_t)nReadLen + kPRE_DEC_SIZE : (uint32_t)nReadLen); }

      // update send data
      //pFileNodeFC->SetSendSize(nReadPos + nReadLen);
      pFileNodeFC->UpdateSendSize(nReadLen);
      m_nSyncPos += nReadLen;

      // one file one send
    } while (0);


    if (nPrevSendPos < pFileNodeFC->GetSendSize()) {
      LOG(ILogWriter::LV_TRACE, "Send File Data. ID[%u], Name[%s], %I64u, %I64u"
        , pIFileNode->ID()
        , pIFileNode->LocalPath()
        , pFileNodeFC->GetSendSize()
        , nReadLen
        );
    }

  }

  // send all file data
  if (TRUE == bSendLast) {
    reqFileData(IFileNode::INVALID_FILE_ID, 0, NULL, 0);
    return RC_S_OK;
  }

  return RC_S_NOTFOUND;
}

//////////////////////////////////////////////////////////////////////////
//

void LogCollectorImpl::setMemMiniLogIncludeLog(const char_t* strIncludeLog) {

  const char_t* strFind = NULL;

  do {

    m_listMemMiniLogIncludeLog.push_back(includt_log());
    includt_log& il = m_listMemMiniLogIncludeLog.back();

    strFind = STRCHR(strIncludeLog, STRLEN(strIncludeLog), ',');
    if (NULL == strFind)  {
      STRCPY(il.strText, sizeof(il.strText), strIncludeLog);
      break;
    }

    STRNCPY(il.strText, sizeof(il.strText), strIncludeLog, strFind - strIncludeLog);
    strIncludeLog = strFind + 1;

  } while (*strIncludeLog);
}

void LogCollectorImpl::preProcLogFile(uint32_t* nMemMiniLogData, uint8_t* pLogData, uint32_t nLen) {

  // filter
  if (MEM_NINI_LOG == m_eCollectorType) {

    uint32_t nLogLen = nLen;
    const char_t* strLog = (const char_t*)pLogData;
#define IS_HAVE_INCLUDE_LOG(x)    (x & kHaveLastIncludeLog)
#define SET_HAVE_INCLUDE_LOG(x)   (x |= kHaveLastIncludeLog)
#define UNSET_HAVE_INCLUDE_LOG(x) (x &= ~(kHaveLastIncludeLog))

    // last include log
    static const uint32_t kHaveLastIncludeLog = 1 << 1;
    if (IS_HAVE_INCLUDE_LOG(*nMemMiniLogData)) {

      // get last log
      const char_t* strFindEnd = (const char_t*)MEMCHR(strLog, '\n', nLogLen);
      if (NULL == strFindEnd) {
        // All log is used.
        return; }

      const char_t* strFindEndZero = (const char_t*)MEMCHR(strLog, 0x00, nLogLen);
      if (strFindEndZero && strFindEndZero < strFindEnd) {
        strLog = (const char_t*)strFindEndZero + 1;
      }
      else {
        strLog = strFindEnd + 1;
      }

      // next is data
      strFindEnd = (const char_t*)MEMCHR(strLog, '\n', nLogLen);
      if (strFindEnd && '|' == *strFindEnd) { strLog = strFindEnd + 1; }

      nLogLen = nLen - (strLog - (const char_t*)pLogData);
      UNSET_HAVE_INCLUDE_LOG(*nMemMiniLogData);
    }

    bool_t bIsIncludeLog = FALSE;
    while (strLog < (const char_t*)pLogData + nLen ) {

      // find \r\n
      const char_t* strFindEnd = (const char_t*)MEMCHR(strLog, '\n', nLogLen);
      if (strFindEnd) {

        // fix strlog
        do 
        {
          const char_t* strFindEndZero = (const char_t*)MEMCHR(strLog, 0x00, nLogLen);
          if (NULL == strFindEndZero || strFindEndZero > strFindEnd) { break; }

          nLogLen -= (uint32_t)(strFindEndZero - strLog);
          strLog = strFindEndZero + 1;
        } while(strLog < ((const char_t *)pLogData + nLen));
      }
      else {
        strFindEnd = (const char_t *)pLogData + nLen;
        return;
      }

      //ASSERT(strFindEnd);
      // is data
      if (TRUE == bIsIncludeLog) {
        const char_t* nLastChar = strFindEnd;
        while (' ' == *(nLastChar)
          || '\r' == *(nLastChar)
          || '\n' == *(nLastChar)
          )
        {
          --nLastChar;
        }

        if ('|' == *(nLastChar)) {
          strLog = strFindEnd + 1;
          bIsIncludeLog = FALSE;
          continue;
        }
      }

      // find include log req
      {
        bIsIncludeLog = FALSE;
        include_log_list_t::const_iterator it_list, end;
        for (it_list = m_listMemMiniLogIncludeLog.begin(), end = m_listMemMiniLogIncludeLog.end()
          ; it_list != end; ++it_list
          )
        {
          const includt_log& il = *(it_list);
          if (STRSTR(strLog, strFindEnd - strLog, il.strText)) {
            // save this.
            bIsIncludeLog = TRUE;
            break;
          }
        }

        // clear data
        if (FALSE == bIsIncludeLog) { BZERO(strLog, strFindEnd - strLog + 1); }
      }

      strLog = strFindEnd + 1;
      nLogLen = nLen - (strLog - (const char_t *)pLogData);
    }
  }
}



//////////////////////////////////////////////////////////////////////////
LogCollectorImpl* LogCollectorImpl::CreateInstance(IHostContext* pIHostContext, INetHandler* pINetHandler) {

  return new LogCollectorImpl(pIHostContext, pINetHandler);
}

END_NAMESPACE_COLLECTOR
