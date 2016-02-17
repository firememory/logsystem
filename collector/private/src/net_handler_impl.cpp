/*

*/

#include "net_handler_impl.h"

//#include "aggregator_base.h" // kProtocolID

#include "file_util.h"
#include "thread.h"

//USING_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////
// taapi

#if defined(PLATFORM_API_WINDOWS)
#include <WinSock2.h>
#define USE_WINDOWS_DEF
#endif // PLATFORM_API_WINDOWS

#include "windows_def.h"
#include <taapi.h>

#ifdef _MSC_VER
# pragma warning(disable: 4311)
# pragma warning(disable: 4312)
# pragma comment(lib, "ws2_32.lib")
// # ifdef DEBUG_ON
// #   pragma comment(lib, "taapid.lib")
// # else
// #   pragma comment(lib, "taapi.lib")
// # endif // DEBUG_ON
#endif /* _MSC_VER */

BEGIN_NAMESPACE_COLLECTOR

//////////////////////////////////////////////////////////////////////////

USING_BASE(Thread);
USING_BASE(micro_time);
USING_BASE(OpenFile);
USING_BASE(ReadFile);
USING_BASE(strAttrOpenRead);

USING_NET(NetworkInterface);


#define LOG(lv, ...)          m_pIHostContext->Log(lv, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////
void NetHandlerImpl::Release() {}

rc_t NetHandlerImpl::Open() {

  // connect to aggregator.
  resetJobCount();
  return connectToAggregator();
}

rc_t NetHandlerImpl::Close() {

  return disConnectToAggregator();
}

void NetHandlerImpl::rateControl(uint32_t nCurrentLen) {

  Time now;
  uint32_t nSendRate;

  if (m_bSlowDown) {

    // wait job all finish
    while (m_acJobRecv != m_acJobSend) {
      if (FALSE == m_bRunning) { return; }
      Thread::sleep(10);
    }

    // 4 mutil time
    do 
    {
      now = micro_time::to_millisecond(micro_time::time());
      Thread::sleep(10);
      if (FALSE == m_bRunning) { return; }
    } while(now < m_timeLastSend + m_nKeepAlive);

    nSendRate = m_nSendRate / 3;

    // resume
  }
  else {
    now = micro_time::to_millisecond(micro_time::time());
    nSendRate = m_nSendRate;
  }

  if (0 == nSendRate) { return ;}
  uint32_t nTimeDistance = (uint32_t)(now - m_timeLastSend);

  const uint32_t kSecCount = 1000;
  if (600 >= nTimeDistance) {

    m_nLastSendBytes += nCurrentLen;
    if (m_nLastSendBytes > nSendRate) {

      Thread::sleep(kSecCount - nTimeDistance);
      m_timeLastSend = now;
      m_nLastSendBytes = 0;
    }
    return;
  }

  m_timeLastSend = now;
  m_nLastSendBytes = nCurrentLen;
}

rc_t NetHandlerImpl::Send(const void* context, const uint8_t* data, uint32_t len) {

  //USING_AGGREGATOR(ProtocolID_LSA);
  static const uint16_t ProtocolID_LSA = 9888;

  if (NULL == data || 0 == len) { return RC_S_NULL_VALUE; }

  // rate control  
  rateControl(len);

  if (FALSE == m_bRunning) { return RC_S_STATUS; }

  TRACE("TEST: 原生协议\r\n");

  // XXX
  //uint32_t nJobData = (uint32_t)(context);
  UNUSED_PARAM(context);
  IJob* pOriginJob = NULL;
  pOriginJob = m_Taapi_Engine->CreateJob(m_Taapi_Client, "CTAJob_RAW", 0, 0, ++m_acJobSend);
  if (NULL == pOriginJob) { return RC_E_NOMEM; }
  ++m_nJobSendCount;
  pOriginJob->Set("CmdNo", ProtocolID_LSA);
  pOriginJob->Set("Body", data, len);
  
  pOriginJob->Execute();
  pOriginJob->Release();

  // wait job finish
  const uint32_t kWaitCount = 120 * 10; // 3 min
  uint32_t nWaitCount = 0;
  while (TRUE == m_bRunning && FALSE == IsSendAllOk() && nWaitCount < kWaitCount) {
    ++nWaitCount; Thread::sleep(100);
  }

  if (FALSE == m_bRunning || nWaitCount >= kWaitCount) { return RC_S_FAILED; }

  return RC_S_OK;
}

bool_t NetHandlerImpl::IsSendAllOk() const {

  return TRUE == m_bRunning && m_acJobRecv == m_acJobSend ? TRUE : FALSE;
}

void NetHandlerImpl::SetKeepAlive(uint32_t sec) { m_nKeepAlive = sec; }

void NetHandlerImpl::SetRateControl(uint32_t nKBS, bool_t bSlowDown) {

  //m_nSendRate = nKBS;
  m_nSendRate = (nKBS * 3) / 4;
  m_bSlowDown = bSlowDown;
}

uint32_t NetHandlerImpl::GetSessionID() const { return 0; }

const uint8_t* NetHandlerImpl::GetLocalIP(size_t* len) {

  if (0 == m_listNI.size()) { return NULL; }
  
  const NetworkInterface& ni = m_listNI[0];
  if (len) { (*len) = ni.address.size(); }
  return &ni.address.front();
}

const uint8_t* NetHandlerImpl::GetLocalMAC(size_t* len) {

  if (0 == m_listNI.size()) { return NULL; }

  const NetworkInterface& ni = m_listNI[0];
  if (len) { (*len) = ni.mac.size(); }
  return &ni.mac.front();
}

const uint8_t* NetHandlerImpl::GetTargetIP(size_t* len) {

  if (len) { (*len) = m_ipTarget.size(); }
  return &m_ipTarget.front();
}

const uint8_t* NetHandlerImpl::GetTargetMAC(size_t* len) {

  if (len) { (*len) = m_ipTarget.size(); }
  return &m_ipTarget.front();
}

void NetHandlerImpl::SetNetEvent(INetEvent* NetEvent) {
  m_pINetEvent = NetEvent;
}

void NetHandlerImpl::Set(const char_t* strParam, const void* pData, uint32_t nDataLen) {

  UNUSED_PARAM(nDataLen);
  if (0 == STRCMP(m_strAttrUser, strParam)) {
    const char_t* strUser = (const char_t*)(pData);
    STRCPY(m_strNetUser, sizeof(m_strNetUser), strUser);
  }
  else if (0 == STRCMP(m_strAttrPWD, strParam)) {
    const char_t* strPWD = (const char_t*)(pData);
    STRCPY(m_strNetPWD, sizeof(m_strNetPWD), strPWD);
  }
}

void Set(const char_t* strParam, void* pData, uint32_t nDataLen);
//////////////////////////////////////////////////////////////////////////
const char_t* NetHandlerImpl::m_strAttrUser       = _STR("User");
const char_t* NetHandlerImpl::m_strAttrPWD        = _STR("PWD");

//////////////////////////////////////////////////////////////////////////
// NetHandler
NetHandlerImpl::NetHandlerImpl(IHostContext* pIHostContext, INetEvent* pINetEvent)
  : m_bInit(FALSE)
  , m_bRunning(FALSE)

  , m_pIHostContext(pIHostContext)
  , m_dyncEngine(NULL)
  , m_pfnTaApi_CreateInstance(NULL)

  , m_Taapi_Engine(NULL)
  , m_Taapi_Client(NULL)

  , m_pINetEvent(pINetEvent)

  , m_nJobSendCount(0)
  , m_nJobOKCount(0)
  , m_nJobFailedCount(0)
  , m_acJobFinishCount(0)

  , m_listNI()

  , m_acJobSend(0)
  , m_acJobRecv(0)

  , m_nSendRate(0)
  , m_timeLastSend(0)
  , m_nLastSendBytes(0)
  , m_bSlowDown(FALSE)
{
  // 初始化SOCKET
  WSADATA wsaData;
  int retval = WSAStartup(0x202, &wsaData);
  if (0 != retval) {
    LOG(ILogWriter::LV_ERR, "WSAStartup Failed");
    WSACleanup();
    return;
  }

  GetNetworkList(&m_listNI);
  //m_bLocalIP

  if (NULL == m_pIHostContext) { return; }

#if defined(DEBUG_ON)
  const char_t* strEngineLib  = _STR("taapi.dll");
#else
  const char_t* strEngineLib  = _STR("taapi.dll");
#endif

  const char_t* strTaApi_CreateInstance  = _STR("TaApi_CreateInstance");

  char_t strEngineDir[MAX_PATH] = {0};
  SNPRINTF(strEngineDir, sizeof(strEngineDir), _STR("%s"), m_pIHostContext->GetSystemExeFullPath());
  if (RC_S_OK != m_dyncEngine.open(strEngineLib, strEngineDir)) {
    LOG(ILogWriter::LV_ERR, "Can Not LoadLibrary %s/%s", strEngineDir, strEngineLib);
    return;
  }

  m_pfnTaApi_CreateInstance = m_dyncEngine.LocateSymbol<PFN_TAAPI_CREATEINSTANCE>(strTaApi_CreateInstance);
  if (NULL == m_pfnTaApi_CreateInstance) {
    LOG(ILogWriter::LV_ERR, "Can Not Get ProcessAddr %s/%s %s", strEngineDir, strEngineLib, strTaApi_CreateInstance);
    return;
  }

  BZERO_ARR(m_strNetUser);
  BZERO_ARR(m_strNetPWD);
  m_bInit = TRUE;
}

NetHandlerImpl::~NetHandlerImpl() {

  if (m_Taapi_Engine) { DeInitNet(); }

  if (TRUE == m_bInit) { 
    // 释放SOCKET环境
    WSACleanup();
  }
}

rc_t NetHandlerImpl::InitNet() {

  ASSERT(NULL == m_Taapi_Engine);
  if (TRUE != m_bInit) { return RC_S_STATUS; }

  file_size_t nXMLSize = 4*1024;
  uint8_t XMLBuffer[4*1024] = {0};
  const char_t* strEngineConfigFile = _STR("taapi.xml");
  AutoReleaseFile autoRelFile(OpenFile(strEngineConfigFile, strAttrOpenRead));
  if (NULL == autoRelFile || RC_S_OK != ReadFile(XMLBuffer, &nXMLSize, autoRelFile, 0)) {

    LOG(ILogWriter::LV_ERR, "Can Not Load TAAPI Engine Config File %s", strEngineConfigFile);
    return RC_S_NOTFOUND;
  }

  m_Taapi_Engine = (m_pfnTaApi_CreateInstance)((const char_t*)(XMLBuffer), (uint32_t)(nXMLSize));
  return m_Taapi_Engine ? RC_S_OK : RC_S_FAILED;
}

rc_t NetHandlerImpl::DeInitNet() {

  if (TRUE != m_bInit) { return RC_S_STATUS; }

  waitForJobFinish();
  if (m_Taapi_Client) { m_Taapi_Client->Release(); }

  StopNet();
  m_Taapi_Engine->Release();
  m_Taapi_Engine = NULL;

  return RC_S_OK;
}

rc_t NetHandlerImpl::waitForJobFinish(uint32_t sec) const {

  uint64_t now_time = micro_time::to_second(micro_time::tick());
  uint64_t start_time = now_time;
  while ((uint32_t)m_acJobFinishCount != m_nJobSendCount && (uint32_t)(now_time - start_time) < sec) {

    now_time = micro_time::to_second(micro_time::tick());
    Thread::sleep(1);
  }
  return (uint32_t)(now_time - start_time) < sec ? RC_S_OK : RC_S_FAILED;
}

static const char_t strJobPushing[]          = _STR("CTAJob_Pushing");
static const uint32_t nStrJobPushingLen      = sizeof(strJobPushing) - 1;

void NetHandlerImpl::EngineJobNotify(NetHandlerImpl* pNetHandlerImpl) {

  if (NULL == pNetHandlerImpl) { return; }

  if (NULL == pNetHandlerImpl->m_pINetEvent) {
    IJobQueue* pIJobQueue = pNetHandlerImpl->m_Taapi_Engine->QueryJobQueue(NULL);
    if (NULL == pIJobQueue) { return; }

    IJob* pOriginJob = NULL;
    DWORD dwLastError = 0;
    while (NULL != (pOriginJob = pIJobQueue->GetCompletionJob(1, dwLastError))) {

      const char_t* jobClsName = NULL;
      pOriginJob->Get("ObjClsName", &jobClsName);
      TRACE(jobClsName);

      // skip pushing job
      if (NULL == jobClsName || 0 == STRNCMP(jobClsName, strJobPushing, nStrJobPushingLen)) {
        pOriginJob->Release();
        continue;
      }

      LONG errType;
      pOriginJob->Get("ErrType", &errType);
      if (errType) {
        ++pNetHandlerImpl->m_nJobFailedCount;
      }
      else {
        ++pNetHandlerImpl->m_nJobOKCount;
      }

      ++pNetHandlerImpl->m_acJobFinishCount;
      pOriginJob->Release();
    }
  }

  pNetHandlerImpl->engineJobNotify();
}

void NetHandlerImpl::engineJobNotify() {

  IJob* pOriginJob = NULL;
  DWORD dwLastError = 0;

  void* pContext = NULL;
  IJobQueue* pIJobQueue = m_Taapi_Engine->QueryJobQueue(NULL);
  if (NULL == pIJobQueue) { return; }

  while (NULL != (pOriginJob = pIJobQueue->GetCompletionJob(1, dwLastError))) {

    const char_t* jobClsName = NULL;
    pOriginJob->Get("ObjClsName", &jobClsName);
    TRACE(jobClsName);

    // skip pushing job
    if (NULL == jobClsName || 0 == STRNCMP(jobClsName, strJobPushing, nStrJobPushingLen)) {
      pOriginJob->Release();
      continue;
    }

    DWORD dwJobData = 0;
    pOriginJob->Get("JobData", &dwJobData);
    // XXX
    pContext = (void *)(dwJobData);

    LONG errType = 0;
    pOriginJob->Get("ErrType", &errType);
    if (errType) {
      ++m_nJobFailedCount;

      // failed
      const char_t* errInfo = NULL;
      pOriginJob->Get("ErrInfo", &errInfo);

      uint32_t nErrCode = 0;
      pOriginJob->Get("ErrCode", &nErrCode);
      TRACE(errInfo);

      LOG(ILogWriter::LV_ERR, "Net Error. %u, %s", nErrCode, errInfo);

      if (0 == STRCMP(jobClsName, "CTAJob_Closed")) {

        m_bRunning = FALSE;

        m_pINetEvent->NetEvent(pContext, INetEvent::DISCONNECT, 0, NULL);

        //
        pOriginJob->Release();
        continue;
      }
      else {
        m_pINetEvent->NetEvent(pContext, INetEvent::RESPONSE_STATUS, errType, (const uint8_t*)(errInfo));
      }
    }
    else {

      ++m_nJobOKCount;
      /*
      const char_t strUnsupportJob[] = _STR("CTAJob_Pushing");
      if (0 == STRNCMP(jobClsName, strUnsupportJob, sizeof(strUnsupportJob))) {
        // 
        pOriginJob->Release(); continue;
      }
      if (0 == STRCMP(jobClsName, "CTAJob_Open")) {
        m_pINetEvent->NetEvent(pContext, INetEvent::CONNECTED, 0, NULL);
      }
      else if (0 == STRCMP(jobClsName, "CTAJob_InetTouch")
        || 0 == STRCMP(jobClsName, "Job_InetSSLShake")
        ) {
          TRACE("JOB = %s", jobClsName);
      }
      */
      if (0 == STRCMP(jobClsName, "CTAJob_RAW")) {

        // We Need Job
        uint8_t* lpData = NULL;
        uint32_t nDataLen = 0;
        pOriginJob->Get("Body", &lpData, &nDataLen);

        if (lpData && nDataLen) {
          m_pINetEvent->NetEvent(pContext, INetEvent::RESPONSE_DATA, nDataLen, lpData);
        }

        ++m_acJobRecv;
      }
      else if (0 == STRCMP(jobClsName, "CTAJob_OpenEx")) {
        m_pINetEvent->NetEvent(pContext, INetEvent::CONNECTED, 0, NULL);
      }
      else if (0 == STRCMP(jobClsName, "CTAJob_Close")) {}
      else {

        // unsupport job
        LOG(ILogWriter::LV_WARN, "Recv UnSupport Packet. %s", jobClsName);
      }
    } // OK

    // skip pushing job

    ++m_acJobFinishCount;
    pOriginJob->Release();
  }
}

//////////////////////////////////////////////////////////////////////////
void NetHandlerImpl::setNetUser(const char_t* strUser, const char_t* strPWD) {

  STRCPY(m_strNetUser, sizeof(m_strNetUser), strUser);
  STRCPY(m_strNetPWD, sizeof(m_strNetPWD), strPWD);
}

static const char_t* strClientType_MC = "CT_INET";

rc_t NetHandlerImpl::StartNet() {

  ASSERT(m_Taapi_Engine);
  if (TRUE != m_bInit) { return RC_S_STATUS; }

  DWORD dwVersion = 0;
  m_Taapi_Engine->Get(_STR("Version"), &dwVersion);
  LOG(ILogWriter::LV_INFO, "TAAPI Engine Version. %u", dwVersion);
  const uint32_t kMIN_ENGINE_VER = 10100008;
  if (dwVersion < kMIN_ENGINE_VER) {
    LOG(ILogWriter::LV_ERR, "TAAPI Engine Version Is UnSupport. We Need Big Then 10100008");
    return RC_S_FAILED;
  }

  DWORD dwReqBufSize = 0;
  m_Taapi_Engine->Get(_STR("ReqBufSize"), &dwReqBufSize);
  LOG(ILogWriter::LV_INFO, "TAAPI Engine ReqBufSize. %u", dwReqBufSize);
  if (dwReqBufSize < INetHandler::kMAX_REQ_DATA_SIZE) {
    LOG(ILogWriter::LV_ERR, "TAAPI Engine ReqBufSize Is Too Small. We Need More Then 65535");
    return RC_S_FAILED;
  }
  
  char_t szError[MAX_PATH]={0};
  if(!m_Taapi_Engine->Startup(szError,sizeof(szError))) {
    m_Taapi_Engine->Shutdown(0);
    m_Taapi_Engine->Clearup();
    TRACE(szError);

    LOG(ILogWriter::LV_ERR, "TAAPI Engine Start Failed. %s", szError);
    return RC_S_FAILED;
  }

  // create client
  m_Taapi_Client = m_Taapi_Engine->CreateClient(strClientType_MC, 0);
  if (NULL == m_Taapi_Client) {

    LOG(ILogWriter::LV_ERR, "TAAPI Engine CreateClient Failed");
    return RC_S_NULL_VALUE;
  }

  m_Taapi_Client->Set(_STR("IdentityInfo")
    , _STR("file.collector")
    , _STR("collector.v1.0.914")
    , 40//(WORD)CLIENTTYPE_CRM
    , _STR("1.0.914")
    , (WORD)1234
    , (BYTE)1
    , 2//(BYTE)ENCRYPTLV_SSL
    , (BOOL)TRUE
    , 2//(BYTE)PKI_ECC
    , _STR("tdx")
    , m_strNetUser
    , m_strNetPWD
    );

  // set CallBack
  m_Taapi_Engine->Set("JobNotify", EngineJobNotify, this);
  m_bRunning = TRUE;
  return RC_S_OK;
}

void  NetHandlerImpl::resetJobCount() {

  m_nJobSendCount     = 0;
  m_nJobOKCount       = 0;
  m_nJobFailedCount   = 0;
  m_acJobFinishCount  = 0;
}

rc_t NetHandlerImpl::StopNet() {

  ASSERT(m_Taapi_Engine);

  m_bRunning = FALSE;
  if (TRUE != m_bInit) { return RC_S_STATUS; }

  m_Taapi_Engine->Shutdown(0);
  m_Taapi_Engine->Clearup();

  return RC_S_OK;
}

rc_t NetHandlerImpl::connectToAggregator() {

  resetJobCount();

  IJob* pOriginJob = NULL;
  pOriginJob = m_Taapi_Engine->CreateJob(m_Taapi_Client, "CTAJob_OpenEx");
  if (NULL == pOriginJob) { LOG(ILogWriter::LV_ERR, "Create CTAJob_OpenEx Failed"); return RC_E_NOMEM; }
  ++m_nJobSendCount;
  pOriginJob->Execute();
  pOriginJob->Release();

  TRACE("TEST: 等待连接打开\r\n");
  if (RC_S_OK != waitForJobFinish() || m_nJobSendCount != m_nJobOKCount) {

    LOG(ILogWriter::LV_ERR, "Exec CTAJob_OpenEx Failed");
    return RC_S_FAILED;
  }

  /*
  IJob* pOriginJob = NULL;
  pOriginJob = m_Taapi_Engine->CreateJob(m_Taapi_Client, "Job_Open");
  if (NULL == pOriginJob) { LOG(ILogWriter::LV_ERR, "Create Job_Open Failed"); return RC_E_NOMEM; }
  ++m_nJobSendCount;
  pOriginJob->Execute();
  pOriginJob->Release();

  TRACE("TEST: 等待连接打开\r\n");
  if (RC_S_OK != waitForJobFinish() || m_nJobSendCount != m_nJobOKCount) {

    LOG(ILogWriter::LV_ERR, "Exec Job_Open Failed");
    return RC_S_FAILED;
  }

  pOriginJob = m_Taapi_Engine->CreateJob(m_Taapi_Client, "Job_InetTouch");
  if (NULL == pOriginJob) { LOG(ILogWriter::LV_ERR, "Create Job_InetTouch Failed"); return RC_E_NOMEM; }
  ++m_nJobSendCount;
  pOriginJob->Execute();
  pOriginJob->Release();

  TRACE("TEST: 等待握手\r\n");
  if (RC_S_OK != waitForJobFinish() || m_nJobSendCount != m_nJobOKCount) {

    LOG(ILogWriter::LV_ERR, "Exec Job_InetTouch Failed");
    return RC_S_FAILED;
  }

  pOriginJob = m_Taapi_Engine->CreateJob(m_Taapi_Client, "Job_InetSSLShake");
  if (NULL == pOriginJob) { LOG(ILogWriter::LV_ERR, "Create Job_InetSSLShake Failed"); return RC_E_NOMEM; }
  ++m_nJobSendCount;
  pOriginJob->Execute();
  pOriginJob->Release();

  TRACE("TEST: 等待SSL握手\r\n");
  //if (RC_S_OK != waitForJobFinish() || m_nJobSendCount != m_nJobOKCount) { return RC_S_FAILED; }
  if (RC_S_OK != waitForJobFinish() && (uint32_t)m_acJobFinishCount != m_nJobOKCount) {
    
    LOG(ILogWriter::LV_ERR, "Exec Job_InetSSLShake Failed");
    return RC_S_FAILED;
  }
  */

  m_acJobSend = 0;
  m_acJobRecv = 0;
  m_bRunning = TRUE;
  return RC_S_OK;
}

rc_t NetHandlerImpl::disConnectToAggregator() {

  IJob* pOriginJob = NULL;
  pOriginJob = m_Taapi_Engine->CreateJob(m_Taapi_Client, "CTAJob_Close");
  if (NULL == pOriginJob) { LOG(ILogWriter::LV_ERR, "Create CTAJob_Close Failed"); return RC_E_NOMEM; }
  ++m_nJobSendCount;
  pOriginJob->Execute();
  pOriginJob->Release();

  TRACE("TEST: 等待连接断开\r\n");
//   if (RC_S_OK != waitForJobFinish() || m_nJobSendCount != m_nJobOKCount) { return RC_S_FAILED; }
//   return RC_S_OK;
  return waitForJobFinish();
}

END_NAMESPACE_COLLECTOR

