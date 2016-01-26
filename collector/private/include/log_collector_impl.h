/*

*/

#ifndef LOG_COLLECTOR_IMPL_H_
#define LOG_COLLECTOR_IMPL_H_

#include "collector.h"
#include "collector_export.h"

#include "host_context.h"
#include "net_handler.h"
#include "collector_config.h"

#include "file_collector.h"

#include "memory_pool.h"
#include "std_list"
#include "time_util.h"

#include "coder.h"

BEGIN_NAMESPACE_COLLECTOR

USING_AGGREGATOR(INetEvent);
USING_AGGREGATOR(INetHandler);
USING_AGGREGATOR(IHostContext);
USING_AGGREGATOR(IFileNode);
USING_AGGREGATOR(file_id_t);
USING_AGGREGATOR(kMAX_NAME_LEN);

//USING_AGGREGATOR(CollectorConfig);
// USING_BASE(MemoryPoolThreadSafe);
USING_BASE(MemoryNodeStd);
USING_BASE(micro_time);
USING_BASE(DayTime);
USING_BASE(Time);
USING_BASE(TimeControl);
USING_BASE(Lock);

USING_CODER(ICoder);

class RequestAndResponse;

class LogCollectorImpl : public INetEvent {

  // INetEvent
public:
  rc_t NetEvent(const void* context, net_event_e type, uint32_t len, const uint8_t* data);

public:
  LogCollectorImpl(IHostContext* pIHostContext, INetHandler* pINetHandler);
  ~LogCollectorImpl();

public:
  rc_t Start();
  rc_t Stop();
  rc_t ReStart();
  rc_t Run();

public:
  void Release() { delete this; }
  static LogCollectorImpl* CreateInstance(IHostContext* pIHostContext, INetHandler* pINetHandler);

private:
  rc_t init();

  // xmlconfig
private:
  const char_t*       m_strLastEL;
  rc_t defaultConfig();
  rc_t readConfig();
  static void expatStart(void *data, const char *el, const char **attr);
  static void expatEnd(void *data, const char *el);
  bool_t m_bConfigName;

private:
  rc_t idle();
  rc_t sendRequestFileNode();
  rc_t collectFile(uint32_t);
  uint32_t m_timeLastCollect;
  uint32_t m_timeLastCollectControl;

  inline void setCollector(const char_t* strCollector) { 
    ASSERT(strCollector); 
    STRCPY(m_strCollector, kMAX_NAME_LEN, strCollector);
  }

  inline void setAggregator(const char_t* strAggregator) { 
    ASSERT(strAggregator); 
    STRCPY(m_strAggregator, kMAX_NAME_LEN, strAggregator);
  }

  // config
private:
  CollectorConfig             m_CollectorConfig;
  CollectorConfig& getCollectorConfig() { return m_CollectorConfig; }

  // network
private:
  typedef uint32_t      trans_id_t;
  inline void setLogon(bool_t bLogon) {
    m_bLogon = bLogon; m_trxID = 0;
    m_nAggSyncPos = m_nSyncPos = 0;
  }
  inline bool_t didLogon() { return m_bLogon; }

  inline trans_id_t TransID() { return ++m_trxID; }

  trans_id_t        m_trxID;
  volatile bool_t   m_bLogon;
  //Lock              m_lockLogCollector;

  //micro_time
  char_t            m_strAggregator[kMAX_NAME_LEN + 1];
  char_t            m_strCollector[kMAX_NAME_LEN + 1];

  // packet
private:
  rc_t reqLogon();
  rc_t reqLogout(const char_t* strMsg = NULL);
  rc_t reqKeepAlive();
  rc_t reqLog(uint32_t type, const char_t* strExtern, uint8_t* data = NULL, uint32_t len = 0);
  rc_t reqConfig(bool_t bSend);
  rc_t reqCollectorFile(bool_t bSend);
  rc_t reqCollectRule(bool_t bSend);
  rc_t reqFile(const char_t* strDir, const char_t* strName, uint64_t create_time);
  rc_t reqFileData(file_id_t file_id, file_size_t pos, uint8_t* data, uint32_t len);
  rc_t reqCheckSum(file_id_t file_id, uint32_t fb_cb_pos, uint32_t fb_cs_count);

  rc_t reqAllConfig();

private:
  rc_t packetProc(uint32_t, uint32_t len, const uint8_t* data);
  rc_t packetProcLogon(RequestAndResponse*);
  rc_t packetProcLogout(RequestAndResponse*);
  rc_t packetProcKeepAlive(RequestAndResponse*);

  rc_t packetProcLog(RequestAndResponse*);
  rc_t packetProcConfig(RequestAndResponse*);
  rc_t packetProcCheckSum(RequestAndResponse*);

  rc_t packetProcCollectorFile(RequestAndResponse*);
  rc_t packetProcCollectRule(RequestAndResponse*);
  rc_t packetProcInstruct(RequestAndResponse*);

  rc_t packetProcFile(RequestAndResponse*);
  rc_t packetProcFileData(RequestAndResponse*);

  // 
  RequestAndResponse*                   m_pRequestAndResponse;
  rc_t initRequestAndResponse();
  void releaseRequestAndResponse();
  inline bool_t didRequestAndResponse() { return m_pRequestAndResponse ? TRUE : FALSE; }
  inline RequestAndResponse* getResponse(uint32_t /*nUniqueID*/) { return m_pRequestAndResponse; }
  inline RequestAndResponse* getRequest(uint32_t /*nUniqueID*/) { return m_pRequestAndResponse; }

  bool_t m_bPackFileData;


  void checkSumFile(file_id_t);

  //
  uint64_t    m_nSyncPos;
  uint64_t    m_nAggSyncPos;

private:
  rc_t addCollectRule(const char_t* strDir, const char_t* strExclude, const char_t* strInclude);
  void clearCollectRule();
  IFileNode* addFileNode(const file_id_t&, file_size_t pos, bool_t bFinish, const char_t* strDir, const char_t* strName);
  IFileNode* findFileNode(const char_t* strDir, const char_t* strName);
  IFileNode* findFileNode(const file_id_t&);
  rc_t updateFileNode(const file_id_t&, file_size_t size);
  void clearFileNode();
  
  void preProcLogFile(uint32_t*, uint8_t*, uint32_t);
  rc_t fileCollect();
  static rc_t fileCollectCallback(/*IFileCollector*, */const IFileNode*, void* context);

private:
  TimeControl m_tcKeepAlive;
  uint32_t    m_nLastKeepAlive;

  uint32_t    m_nLastSendRate;
  rc_t slowDown();


  TimeControl m_tcTick;
  void tickProc(uint32_t);

  // aggregator file node
private:

  //typedef hash_map<file_id_t, IFileNode*>   file_node_map_t;
  //file_node_map_t             m_mapFileNode;

  //IFileNode* findFileNode(file_id_t);
  //rc_t addMapFileNode(file_id_t, const char_t* strDir, const char_t* strName);
  //rc_t updateMapFileNode(file_id_t, file_size_t);
  //void clearMapFileNode();

  typedef std::queue<IFileNode*>         send_file_node_queue_t;
  send_file_node_queue_t      m_queueSendFileNode;

  rc_t addSendQueue(file_id_t);
  void clearQueueSendFileNode();

private:
  typedef std::queue<const IFileNode*>  req_file_node_queue_t;
  req_file_node_queue_t       m_queueReqFileNode;

  rc_t addReqQueue(const IFileNode*);
  void clearQueueReqFileNode();

  rc_t procFileQueue();

private:
  AutoRelease<ICoder*>        m_autoRelICoderDefalte;
  AutoRelease<MemoryNodeStd*> m_autoRelMemDefalte;
  AutoRelease<MemoryNodeStd*> m_autoRelMemRead;

private:
  volatile rc_t               m_eStatus;
  bool_t                      m_bStop;
  bool_t                      m_bSleep;
  uint32_t                    m_nSleepedTime;

  uint32_t                    m_nConnectWaitTime;

  uint32_t                    m_nCollectorSleepTime;
  bool_t                      m_bStrictMode;

private:
  void setMemMiniLogIncludeLog(const char_t*);

  typedef struct includt_log_t {
    char_t strText[64];
  } includt_log;
  typedef std::list<includt_log_t> include_log_list_t;
  include_log_list_t          m_listMemMiniLogIncludeLog;

  collector_log_type_e        m_eCollectorType;

private:

  AutoRelease<IHostContext*>  m_autoRelIHostContext;
  AutoRelease<INetHandler*>   m_autoRelINetHandler;
  AutoRelease<IFileCollector*> m_autoRelIFileCollector;

  DISALLOW_COPY_AND_ASSIGN(LogCollectorImpl);
}; // LogCollectorImpl

#define STR_VERSION           "v1.0.0.1024"

#define LOG(lv, ...)          m_autoRelIHostContext->Log(lv, __VA_ARGS__)

END_NAMESPACE_COLLECTOR
#endif // LOG_COLLECTOR_IMPL_H_
