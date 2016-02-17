/*

*/

#ifndef AGGREGATOR_IMPL_H_
#define AGGREGATOR_IMPL_H_

// logsystem
#include "aggregator_locl.h"

#include "host_context.h"
#include "database_impl.h"

#include "filesystem.h"
#include "fileiterator_impl.h"

#include "log_reader.h"
#include "collector_config.h"

// base
#include "atomic_count.h"
#include "dynamic_library.h"
#include "pool.h"
#include "thread.h"
#include "hash_tables.h"
#include "time_util.h"

#include "std_list"

// coder
#include "coder.h"

// database
#include "db_utility.h"

//////////////////////////////////////////////////////////////////////////
BEGIN_NAMESPACE_AGGREGATOR

USING_BASE(atomic_count);
USING_BASE(Thread);
USING_BASE(ThreadGroup);
USING_BASE(Lock);
USING_BASE(AutoLock);

USING_BASE(micro_time);
USING_BASE(DayTime);
USING_BASE(TimeControl);

USING_BASE(hash_map);
USING_BASE(char_t_hash);

USING_BASE(DynamicLibrary);

USING_BASE(PoolNoThreadSafe);


USING_CODER(ICoder);

class RequestAndResponse;

class AggregatorImpl : public IAggregator, public Thread, public IResource {

  // IAggregator
// public:
//   void Release() { delete this; }

public:
  rc_t RegisterFunc(IObject*, const char_t*);
  rc_t UnRegisterFunc(IObject*, const char_t*);

public:
  IFileIterator* GetFileIterator(const char_t* strType, uint32_t nWaitTime);
  IDBConnection* GetDBConnection(const char_t* strType, uint32_t nWaitTime);
  ILogWriter* GetLogWriter(uint32_t nWaitTime);

  const char_t* GetSystemFullPath();

  // setting
public:
  rc_t SetSetting(char_t const* name, const IGetSetParam*);
  rc_t GetSetting(IGetSetParam*, char_t const* name);

  //
  IFileContainer* GetFileContainer(const char_t* strType) { return getFileContainer(strType); }

public:
  rc_t Init(IHostContext*);
  void DeInit();
  rc_t Start();
  void Stop();

public:
  uint32_t GetTick();
  uint32_t GetLongTick();

  rc_t TickProc();
  rc_t LongTickProc();
  rc_t NetProc(INetHandler*, const uint8_t* data, uint32_t len);

public:
   // new file
  rc_t AddFile(const file_id_t& file_id, const char_t* collector, const char_t* strDir, const char_t* strName, uint64_t create_time);
  rc_t SetData(const file_id_t& file_id, IMemoryNode* memNode, file_size_t pos, file_size_t len);

  // old file
  rc_t AddFile(const file_id_t& file_id, const char_t* collector, const char_t* strLocalPath
    , const char_t* strDir, const char_t* strName, uint64_t create_time, file_size_t size, bool_t bFinish, const char_t* checksum);

public:
  IFileNode* GetFileNode(const file_id_t& file_id) { return getFileNode(file_id); }


  // IResource
public:
  const rc_t GetName(char_t*, uint32_t*) const;
  const rc_t GetStatus() const;

  const rc_t GetResourceSize(uint64_t*, uint64_t*) const;
  const rc_t GetLastErr(uint32_t*, char_t*, uint32_t*) const;
  const rc_t GetInfo(char_t*, uint32_t*) const;
  const rc_t GetLastAliveTime(uint32_t*) const;

  DEFINE_REF_COUNT_RELEASE_DEL_SELF;
public:
  AggregatorImpl();
  ~AggregatorImpl();

public:
  inline void SetAggregator(const char_t* strAggregator) { 
    if(strAggregator) { STRCPY(m_strAggregator, kMAX_NAME_LEN, strAggregator); }
  }

  inline void SetCollectorLoaclPath(const char_t* strPath) { 
    if(strPath) { 
      STRCPY(m_strCollectorLoaclPath, kMAX_NAME_LEN, strPath);
      if (m_autoRelIFileSystem) { m_autoRelIFileSystem->SetRootDir(m_strCollectorLoaclPath); }
    }
  }

private:
  // context
  AutoRelease<IHostContext*>     m_autoRelIHostContext;

  char_t            m_strAggregatorDir[kMAX_PATH + 1];
  char_t            m_strAggregator[kMAX_NAME_LEN + 1];
  char_t            m_strCollectorLoaclPath[kMAX_NAME_LEN + 1];
  uint32_t          m_timeStart;
  uint32_t          m_timeAlive;
  // client
public:
  class CollectClient;
  typedef PoolNoThreadSafe<CollectClient, const char_t*>    client_pool_t;

  class CollectClient {
  public:
    void Release() { ASSERT(this); m_poolClient->release(this); }

  public:
    CollectClient(void* pool, const char_t* strName)
      : m_poolClient(static_cast<client_pool_t*>(pool))
      , m_bLogon(FALSE)
      , m_config(m_strName)
      , m_timeLogon(0)
      , m_nTimeLastAlive(0)
      , m_nRecvFileDataReport(0)
      , m_nRecvFileDataReal(0)
      , m_nRecvFileData(0)
      , m_bCollectNow(FALSE)
    {
      BZERO_ARR(m_strName); SetName(strName);
    }
    ~CollectClient() {}

  public:
    inline const char_t* GetName() const { return m_strName; }
    inline uint64_t GetSessionID() { return m_session_id; }
    inline void UpdateSessionID() {
      m_session_id = micro_time::time();
      UpdateKeepAlive();
    }

    inline CollectorConfig& GetCollectorConfig() { return m_config; }

  public:
    inline void SetName(const char_t* strName) { 
      if (strName) {
        STRCPY(m_strName, kMAX_NAME_LEN, strName); UpdateSessionID();
        m_nRecvFileData = m_nRecvFileDataReal = m_nRecvFileDataReport = 0;
      }
    }

    inline void SetRecvFileData(uint64_t pos, uint64_t posReal) { m_nRecvFileData += pos; m_nRecvFileDataReal += posReal; }
    inline uint64_t GetRecvFileData() { return m_nRecvFileData; }
    inline uint64_t GetRecvFileDataReal() { return m_nRecvFileDataReal; }
    inline uint64_t GetRecvFileDataReport() { return m_nRecvFileDataReport; }

    inline void SetLogon(bool_t bLogon) {
      m_bLogon = bLogon;
      if (TRUE == m_bLogon) { m_timeLogon = DayTime::now(); }
    }
    inline bool_t isLogon() const { return m_bLogon; }
    uint32_t GetLogonTime() { return m_timeLogon; }

    inline void SetKeepAlive(uint64_t sync_pos) { m_nTimeLastAlive = DayTime::now(); m_nRecvFileDataReport = sync_pos; }
    inline void UpdateKeepAlive() { m_nTimeLastAlive = DayTime::now(); }
    inline uint32_t GetKeepAlive() { return m_nTimeLastAlive; }

  public:
    bool_t didCollectNow() { 
      bool_t bCollecNow = m_bCollectNow; 
      m_bCollectNow = FALSE;
      return bCollecNow;
    }
    void SetCollectNow() { m_bCollectNow = TRUE; }

  private:
    bool_t          m_bLogon;
    uint32_t        m_timeLogon;
    uint32_t        m_nTimeLastAlive;
    uint64_t        m_nRecvFileData;
    uint64_t        m_nRecvFileDataReport;
    uint64_t        m_nRecvFileDataReal;

    bool_t          m_bCollectNow;

  private:
    client_pool_t*  m_poolClient;
    char_t          m_strName[kMAX_NAME_LEN + 1];
    CollectorConfig m_config;
    uint64_t        m_session_id;

  public:
    void AddFile(file_id_t file_id) { ASSERT(IFileNode::INVALID_FILE_ID != file_id); m_listFileNode.push_back(file_id); }
    void ClearFile() { m_listFileNode.clear(); }

  public:
    typedef std::list<file_id_t> collecotor_file_list_t;
    collecotor_file_list_t    m_listFileNode;
  }; // CollectClient

private:
  typedef hash_map<const char_t*, CollectClient*, char_t_hash>        client_map_t;
  client_map_t                m_mapClient;

  client_pool_t               m_poolClient;
  Lock                        m_lockClient;

  CollectClient* addClient(const char_t* strCollector);
  CollectClient* findClient(const char_t* strCollector);
  rc_t removeClient(const char_t* strCollector);
  rc_t initClient();
  void clearClient();

  void tickProc_Client();

  // packet
private:
  CollectClient* checkLogon(rc_t&, const char_t* strCollector, bool_t bSet);

  enum { kDefaultPacketMemorySize = 1* 1024 * 1024 };

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
  rc_t packetProcCollectNow(RequestAndResponse*);

  rc_t packetProcFile(RequestAndResponse*);
  rc_t packetProcFileData(RequestAndResponse*);

  //rc_t addRequestData(uint32_t nUniqueID);
  void removeRequestData(uint32_t nUniqueID);
  RequestAndResponse* getRequestData(uint32_t nUniqueID);

  IMemoryNode* getResponsePacketData(uint32_t nUniqueID);
  typedef hash_map<uint32_t, RequestAndResponse*>    packet_data_map_t;
  packet_data_map_t           m_mapRequestData;

  typedef std::list<RequestAndResponse*>             Request_list_t;
  Request_list_t              m_listRequest; // cache;
  Lock                        m_lockRequestData;

  friend class RequestAndResponse;
  // mock pool
  rc_t get(RequestAndResponse**, uint32_t flag = 0);
  void release(RequestAndResponse*);
  RequestAndResponse* get_nolock();
  void release_nolock(RequestAndResponse*);

  RequestAndResponse* createRequestData(uint32_t nUniqueID);
  rc_t initRequestData();
  void clearRequestData();
//   void*                       m_pRequestData;
//   rc_t initPackData();
//   void releasePackData();
//  bool_t didPackData() { return m_pRequestData ? TRUE : FALSE; }

private:
  rc_t makeFileID(file_id_t& file_id, const char_t* collector, const char_t* dir, const char_t* name, uint64_t create_time);

  atomic_count  m_acFileID;

  typedef enum EncodeType_E_{NONE, DEFLATE} EncodeType_e;
  rc_t updateFileData(ICoder*, const file_id_t&, file_size_t pos, EncodeType_e type, uint32_t olen, uint32_t len, const uint8_t* data);

  IFileNode* getFileNode(const file_id_t&);

  rc_t logDB(ILogWriter::log_lv_t lv, const char_t* from
    , const uint8_t* data, uint32_t dlen
    , const uint8_t* content, uint32_t clen);


private:
  // atomic_count
  typedef hash_map<file_id_t, atomic_count>     file_update_map_t;
  file_update_map_t   m_mapFileUpdate;
  //Lock                m_lockMapFileUpdate;

  TimeControl       m_tcFileUpdate;
  
  void reportMapFileUpdate(uint32_t now);
  void clearMapFileUpdate();
  void updateMapFileUpdate();

private:
  void addLogTypes();
  rc_t setLogTypeFile(const file_id_t&);

  // Module
private:
  rc_t m_eModuleStatus;
  Lock m_lockModule;

private:
  struct module_t {
    DynamicLibrary*             dyncLib;
    PFN_MODULE_CREATE_INSTANCE  pfn_mci;
    IModule*                    module;
  };
  typedef std::list<module_t> module_dynclib_t;
  module_dynclib_t  m_lstModule;

  typedef std::list<IObject*> object_list_t;
  object_list_t     m_lstLogTypeObject;
  object_list_t     m_lstSettingObject;
  object_list_t     m_lstParseObject;

  // thread
private:


  typedef struct st_thread_param {

    typedef enum thread_type_e_ {MODULE, PARSER} thread_type_e;

    thread_type_e type;
    union {
      IModule* pIModule;
      IParser* pIParser;
    };
    uint32_t idx;
/*
    st_thread_param(thread_type_e type, IModule* pIModule)
      : type(type)
      , pIModule(pIModule)
    {}

    st_thread_param(thread_type_e type, IParser* pIParser)
      : type(type)
      , pIParser(pIParser)
    {}
*/
    void Set(thread_type_e type_, IModule* pIModule_, uint32_t idx_) {
      type      = type_;
      pIModule  = pIModule_;
      idx       = idx_;
    }

    void Set(thread_type_e type_, IParser* pIParser_) {
      type      = type_;
      pIParser  = pIParser_;
      idx       = 0;
    }
  } thread_param_t;

  typedef std::list<thread_param_t>     thread_param_list_t;
  thread_param_list_t             m_listThreadParam;

  typedef ThreadGroup<AggregatorImpl>   ThreadGroup_AggregatorImpl_t;
  ThreadGroup_AggregatorImpl_t    m_thdgrpAggregatorImpl;
  static const char_t*        m_strParserThdName;

  void ThreadMain(void*);

  rc_t initModule();
  void clearModule();
  
private:
  // database
  typedef hash_map<const char_t*, DBConn_pool_t*, char_t_hash>    dbconn_pool_map_t;
  dbconn_pool_map_t m_mapDBConnPool;
  uint32_t          m_nPoolDBConnSize;

  typedef std::list<db_conn_str_t>    db_conn_str_list_t;
  db_conn_str_list_t  m_listDBConnStr;

  TimeControl       m_tcDataBase;
  rc_t initDatabase();
  void clearDatabase();
  void tickProc_Database(uint32_t);

  void getDBConnPoolSize(uint32_t* total_size, uint32_t* free_size) const {
    ASSERT(total_size && free_size);

    (*total_size) = (*free_size) = 0;
    dbconn_pool_map_t::const_iterator it_map, end;
    for (it_map = m_mapDBConnPool.begin(), end = m_mapDBConnPool.end(); it_map != end; ++it_map) {

      DBConn_pool_t* pDBConnPool = it_map->second;
      if (NULL == pDBConnPool) { continue; }

      (*total_size) += pDBConnPool->total_size();
      (*free_size) += pDBConnPool->free_size();
    }
  }

  // file system
public:
  void SetFileSystemCacheSize(size_t size);
private:
  size_t            m_filesystem_buffer_size;
  AutoRelease<IFileSystem*>      m_autoRelIFileSystem;
  Lock              m_lockFileSystem;

  rc_t initFileSystem();
  void clearFileSystem();

  // encode
private:
  rc_t initEncodeSystem();
  void clearEncodeSystem();

  // log type
private:
  //////////////////////////////////////////////////////////////////////////
  class LogTypeIdx {
  public:

    typedef std::vector<const char_t*>   logtype_idx_vector_t;
    logtype_idx_vector_t  m_vecIdx;
    //uint32_t              m_last_pos;

    
  public:
    LogTypeIdx(size_t size)
      : m_vecIdx(size)
      //, m_last_pos(0)
    {
      for (size_t idx = 0, end = m_vecIdx.size(); idx != end; ++idx) {
        m_vecIdx[idx] = NULL;
      }
    }

    ~LogTypeIdx() { m_vecIdx.clear(); }
  };
  // file logtype
  
  typedef hash_map<file_id_t, LogTypeIdx*>   file_logtype_map_t;
  file_logtype_map_t  m_mapFileLogType;

  rc_t initLogType();
  void clearLogType();

  // fileiterator
private:
  void notifyFileIterator();
  rc_t notifyFileIterator(const file_id_t&);

  //////////////////////////////////////////////////////////////////////////
  class FileContainerImpl : public IFileContainer {

    // IFileContainer
  public:
    void Release() { ASSERT(m_acRefCount); --m_acRefCount; }
    void AddRef() { ASSERT(kuint32max > m_acRefCount); ++m_acRefCount; }

  public:
    rc_t AttachFileIterator(IFileIterator* pIFileIterator) {
      if (m_mapFileIterator.end() != m_mapFileIterator.find(pIFileIterator)) { return RC_S_DUPLICATE; }
      m_mapFileIterator[pIFileIterator] = pIFileIterator;
      return RC_S_OK;
    }

    rc_t DetachFileIterator(IFileIterator* pIFileIterator) {

      if (m_mapFileIterator.end() == m_mapFileIterator.find(pIFileIterator)) { return RC_S_NOTFOUND; }
      m_mapFileIterator.erase(pIFileIterator);
      return RC_S_OK;
    }

  public:
    /*
    const file_list_t& Container() {
      return m_lstFile;
    }
    */
    rc_t CopyContainer(file_id_t* pFC, size_t* nSize) {

      if (NULL == pFC || NULL == nSize) { return RC_S_NULL_VALUE; }

      AutoLock autoLock(m_lockFC);

      size_t idx = 0;
      IFileContainer::file_list_t::const_iterator it_list, end;
      for (it_list = m_lstFile.begin(), end = m_lstFile.end(); idx < (*nSize) && it_list != end; ++idx, ++it_list) {

        pFC[idx] = *(it_list);
      }

      *(nSize) = m_lstFile.size();
      return RC_S_OK;
    }

  public:
    FileContainerImpl()
      : m_acRefCount(0)
      , m_lstFile()
      , m_mapFileIterator()
      , m_lockFC()
    {}

    ~FileContainerImpl() {

      ASSERT(0 == m_acRefCount);
      Notify();
      m_lstFile.clear();
    }

    void AddFile(const file_id_t& file_id) {
      AutoLock autoLock(m_lockFC);
      m_lstFile.push_back(file_id); }

    void Notify(void) {

      // signal file iterator
      file_iterator_map_t::iterator it_map, end;
      for (it_map = m_mapFileIterator.begin(), end = m_mapFileIterator.end(); it_map != end; ++it_map) {

        if (NULL == it_map->second) { continue; }
        it_map->second->Signal();
      }
    }

  private:
    atomic_count            m_acRefCount;
    file_list_t             m_lstFile;

    typedef hash_map<IFileIterator*, IFileIterator*> file_iterator_map_t;
    file_iterator_map_t     m_mapFileIterator;

    Lock                    m_lockFC;
  }; // FileContainerImpl

  typedef hash_map<const char_t*, FileContainerImpl*, char_t_hash>  logtype_map_t;
  logtype_map_t     m_mapLogType;

  // 
  FileIterator_pool_t m_poolFileIterator;

  FileContainerImpl* getFileContainer(const char_t* strType);
  rc_t addLogType(const char_t* strLogType);
  rc_t addLogTypeFile(const char_t* strLogType, const file_id_t&);

  rc_t initFileIterator();
  void clearFileIterator();

  //////////////////////////////////////////////////////////////////////////
  // count
public:
  void SetStepTimeResourceStatus(uint32_t time) { if (time) { m_tcResourceStatus.SetTimeOut(time); } }
  void SetStepTimeClientStatus(uint32_t time) { if (time) { m_tcClientStatus.SetTimeOut(time); } }
  void SetStepTimeRPCInvoke(uint32_t time) { if (time) { m_tcRPCInvoke.SetTimeOut(time); } }
  void SetStepTimeUpdateFile(uint32_t time) { if (time) { m_tcFileUpdate.SetTimeOut(time); } }

  void SetStepTimeCheckPoint(uint32_t time) { if (time) { m_tcCheckPoint.SetTimeOut(time); } }  

private:
  // count
  uint32_t          m_nCountFailedGetDBConn;  // db
  TimeControl       m_tcReportCount;

  void reportCount(uint32_t now);

  //////////////////////////////////////////////////////////////////////////
  // Resource
  TimeControl       m_tcResourceStatus;

  rc_t reportResourceStatus(const IResource*);
  void reportResourceStatus(uint32_t now);

  //////////////////////////////////////////////////////////////////////////
  // rpc status
private:
  TimeControl       m_tcClientStatus;
  void reportClientStatus(uint32_t time);

  //////////////////////////////////////////////////////////////////////////
  // rpc
  TimeControl       m_tcRPCInvoke;

  struct RPCInvokeNode {
    uint32_t id;
    char_t   from[64];
    char_t   target[64];
    char_t   name[64];
    uint8_t  param[1024];
  };

  typedef std::list<RPCInvokeNode>      rpc_node_list_t;
  rpc_node_list_t   m_listRPCNode;

  void reportRPC(uint32_t);
  rc_t reportRPC(uint32_t id, rc_t err_no, const char_t* err_msg, const uint8_t* data, uint32_t len);

  rc_t rpcCollectNow();
public:
    rc_t AddRPCInvoke(uint32_t, const char_t*, const char_t*, const char_t*, const uint8_t*, uint32_t);
  //////////////////////////////////////////////////////////////////////////
  // log
private:
  LogReader           m_LogReader;

  rc_t initLog();
  void clearLog();

  // checkpoint
private:
  TimeControl       m_tcCheckPoint;
  void checkPoint(uint32_t);

  rc_t checkSystem();

public:
  void SetWakeupParserTime(uint32_t time) { m_nWakeupParserTime = time; }
  void SetLongTickTime(uint32_t time) { m_nLongTickTime = time; }

private:
  uint32_t          m_nWakeupParserTime;
  uint32_t          m_nLongTickTime;

  // config
private:
  const char_t*       m_strLastEL;
  void defaultConfig();
  rc_t readXmlConfig();
  rc_t readSettingConfig();
  rc_t readSettingCollector();
  static void expatStart(void *data, const char *el, const char **attr);
  static void expatEnd(void *data, const char *el);

  DISALLOW_COPY_AND_ASSIGN(AggregatorImpl);
}; // AggregatorImpl


#define STR_VERSION           "v1.0.0.1024"

END_NAMESPACE_AGGREGATOR

#endif // AGGREGATOR_IMPL_H_

