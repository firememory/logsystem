/*

*/

#ifndef PARSER_TC50_H_
#define PARSER_TC50_H_

//////////////////////////////////////////////////////////////////////////
#include "aggregator_base.h"


#include "parse_ix.h"

#include "lock.h"
#include "waitable_event.h"
#include "memory_pool.h"
#include "std_list"
#include "hash_tables.h"
#include "atomic_count.h"

#include "tc50_dict.h"
#include "tc50_log.h"

#include "regex.hpp"

#include "database.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////
USING_BASE(hash_map);
USING_BASE(hash_set);
USING_BASE(char_t_hash);
USING_BASE(MemoryNodeStd);

USING_BASE(atomic_count);
USING_BASE(Lock);
USING_BASE(WaitableEvent);

USING_DATABASE(IConnection);

class ParserTC50 : public IParser, public IFileIteratorBase::IFileIteratorCallBack {

  // IObject
public:
  const uuid& GetUUID() const { return m_gcUUID; }
  const void* ConstCast(const uuid& id) const {
    if (m_gcUUID == id) { return this; }
    else if (kUUID_IParser == id) { return (IParser*)(this); }
    return NULL;
  }
  void* Cast(const uuid& id) {
    if (m_gcUUID == id) { return this; }
    else if (kUUID_IParser == id) { return (IParser*)(this); }
    return NULL;
  }

  // IResource
public:
  const rc_t GetName(char_t*, uint32_t*) const;
  const rc_t GetStatus() const;

  const rc_t GetResourceSize(uint64_t*, uint64_t*) const;
  const rc_t GetLastErr(uint32_t*, char_t*, uint32_t*) const;
  const rc_t GetInfo(char_t*, uint32_t*) const;
  const rc_t GetLastAliveTime(uint32_t*) const;

  // IParser
public:  
  void Release() {}

public:
  rc_t Start();
  void Stop();

  // IFileIteratorCallBack
public:
  rc_t Begin(IFileIteratorBase*);
  rc_t FileProc(IFileIteratorBase*, const IFileNode*);
  rc_t End();

public:
  static const char_t* GetName() { return m_strName; }
public:
  ParserTC50();
  ~ParserTC50();

public:
  rc_t Init(IAggregatorBase* pIAggregator);

private:
  bool_t isFileOver(file_id_t);

  // file map
public:
  rc_t AddToFileMap(file_id_t, file_size_t);  

private:

  class LogFileNode {
  public:
    LogFileNode()
      : m_pos(0)
      , m_pIFileNode(NULL)
      , m_nDate(0)
      , m_nFailedCount(0)
      , m_bDispathch(FALSE)
      , m_bHasError(FALSE)
    {}

    LogFileNode(file_size_t pos)
      : m_pos(pos)
      , m_pIFileNode(NULL)
      , m_nDate(0)
      , m_nFailedCount(0)
      , m_bDispathch(FALSE)
      , m_bHasError(FALSE)
    {}

    ~LogFileNode() {}

    LogFileNode(const LogFileNode& other) { copy(other); }

    void operator=(const LogFileNode& other) { copy(other); }

  public:
    file_size_t GetPos() const { return m_pos; }
    void SetPos(file_size_t size) { m_pos = size; }
    void UpdatePos(file_size_t size) { m_pos += size; }

  public:
    void SetFileNode(const IFileNode* pIFileNode);
    const IFileNode* GetFileNode() { return m_pIFileNode; }

  private:
    void copy(const LogFileNode& other) {
      SetPos(other.GetPos());
      m_pIFileNode  = other.m_pIFileNode;
      m_nDate       = other.m_nDate;
      m_nFailedCount= other.m_nFailedCount;
      m_bDispathch  = other.m_bDispathch;
    }

  private:
    file_size_t m_pos;
    const IFileNode* m_pIFileNode;

  public:
    uint32_t m_nDate;
    uint32_t m_nFailedCount;

  public:
    void SetDispatch(bool_t bDispathch) { m_bDispathch = bDispathch; }
    bool_t GetDispatch() const { return m_bDispathch; }

    void SetHasError(bool_t bHasError) { m_bHasError = bHasError; }
    bool_t IsHasError() { return m_bHasError; }

  private:
    bool_t m_bDispathch;
    bool_t m_bHasError;
  };

  typedef hash_map<file_id_t, LogFileNode>    file_map_t;
  file_map_t                  m_mapFile;

  void clearFileMap();

private:
  inline IDBConnection* getDBConn();
  rc_t execSQL(uint32_t, const uint8_t* strSQL);

  rc_t loadParseFile();
  rc_t updateParseFile(uint32_t, file_id_t, file_size_t);
  rc_t setFailedLog(uint32_t, uint32_t, const char_t* strCollector, file_id_t file_id
    , file_size_t file_pos_start, file_size_t file_pos_end, const uint8_t*, uint32_t);

  rc_t logParse_tc50_dict(IFileIteratorBase* pIFileIterator, LogFileNode&, const IFileNode* pIFileNode);
  rc_t logParse_tc50(uint32_t, const IFileNode* pIFileNode, IFileIteratorBase* pIFileIterator, LogFileNode&);

public:
  void SetLogRule(uint32_t nFuncID) {
    if (nFuncID < kMAX_FUNC_ID) { m_arrLogRule[nFuncID] = kLOG_FLAG; }
  }

private:
  rc_t loadLogRule();
  uint8_t m_arrLogRule[kMAX_FUNC_ID];

  bool_t didMathLogRule(uint32_t nFuncID) {
    if (nFuncID < kMAX_FUNC_ID) { return kLOG_FLAG == m_arrLogRule[nFuncID] ? TRUE : FALSE; }
    return FALSE;
  }

private:
  uint32_t                    m_nwtGetFileIterator;
  uint32_t                    m_nwtGetDBConnection;

  // thread
public:
  enum {kMAX_THEARD_NUM = 8 };

  uint32_t GetThreadNum() const { return m_nThreadNum; }
  void ThreadProc(uint32_t);

  void SetThreadNum(uint32_t nThreadNum) {
    nThreadNum = nThreadNum % kMAX_THEARD_NUM;
    m_nThreadNum =  nThreadNum ? nThreadNum : 1;
  }

  void SetTransformRows(uint32_t nTransformRows) {
    m_nTransformRows = nTransformRows;
  }

private:
  rc_t loadConfig();

  uint32_t m_nThreadNum;
  atomic_count m_nRealThreadNum;
  void waitThread();
  void wakeUpThread();

  class ParserThread {
  public:
    ParserThread();
  public:
    bool_t IsValid() { return m_autoRelMemSQL ? TRUE : FALSE; }
  public:
    WaitableEvent     m_ev;
    TC50Log           m_TC50Log;

    AutoRelease<IConnection*> m_autoRelIDBConn;
    AutoReleaseMemoryBase m_autoRelMemSQL;
    uint32_t          m_nSQLSize;
  }; // ParserThread

  typedef std::queue<LogFileNode*>    file_node_queeu_t;
  file_node_queeu_t   m_queueFileNode;
  Lock                m_lockQueue;

  LogFileNode* popQueueFileNode();
  void pushQueueFileNode(LogFileNode*);

  atomic_count        m_acThread;
  IFileIteratorBase*  m_pIFileIteratorBase;

  ParserThread        m_PT[kMAX_THEARD_NUM];

public:
  //
  void CheckPoint();
  void SetScheduleTime(uint32_t nTime) { m_nTimeSchedule = nTime; }

  void SetHisDBName(const char_t* strName) { if (strName) { STRCPY(m_strHisDBName, sizeof(m_strHisDBName), strName); } }

  void SetScheduleEnable(bool_t bEnable) { m_bScheduleEnable = bEnable; }

  void SetHisDBEng(const char_t* strName) { if (strName) { STRCPY(m_strDBEng, sizeof(m_strDBEng), strName); } }

  void SetHisDBRemote(bool_t bDBRemote) { m_bDBRemote = bDBRemote; }

private:
  Lock                m_lockCheckPoint;
  uint32_t            m_nTimeSchedule;
  bool_t              m_bScheduleOnce;
  bool_t              m_bScheduleEnable;

  char_t              m_strHisDBName[64];

  char_t              m_strDBEng[64];

  bool_t              m_bDBRemote; // remote load data

  // dict
public:
  rc_t AddDict(const char_t* strCollector, const char_t* strDir, file_id_t file_id, uint64_t create_time) {
    //return addDict(strCollector, strDir, file_id, create_time);
    return m_TC50Dict.AddDict(strCollector, strDir, file_id, create_time);
  }

private:

    typedef std::size_t       hash_value_t;
  class DictNode {
  public:
    DictNode(const char_t* strCollector, const char_t* strDir
      , file_id_t file_id, uint64_t create_time, IDictIX* pIDictIX = NULL)
      : m_file_create_time(create_time)
      , m_pIDictIX(pIDictIX)
      , m_hv((hash_value_t)(char_t_hash()(strCollector) + char_t_hash()(strDir) + create_time))
      , m_file_id(file_id)
    {}

    ~DictNode() { if (m_pIDictIX) { m_pIDictIX->Release(); } }

    IDictIX* GetDict() const { return m_pIDictIX; }
    void SetDict(IDictIX* pIDictIX) { m_pIDictIX = pIDictIX; }

    file_id_t GetDictFileID() const { return m_file_id; }

    hash_value_t GetHashValue() const { return m_hv; }
    uint64_t GetFileTime() const { return m_file_create_time; }

  private:
    hash_value_t    m_hv;

    uint64_t        m_file_create_time;   // sec
    file_id_t       m_file_id;

    // 格式是：登录字典长度（4个字节）+登录字典内存+标准字典长度（4个字节）+标准字典内存
    //  吴火生(16888) 16:17:09
    //  文件名：dll名称_券商ID_YYYYMMDDHHMMSS.sto
    IDictIX*        m_pIDictIX;

  };

  struct dict_node_hash {
    enum
    {	// parameters for hash table
      bucket_size = 4,	// 0 < bucket_size
      min_buckets = 8};	// min_buckets = 2 ^^ N, 0 < N

      hash_value_t operator()(const DictNode* pDictNode) const {
        return pDictNode->GetHashValue();
      }

      bool operator()(const DictNode* s1,const DictNode* s2) const {
        return (s1->GetHashValue() == s2->GetHashValue() && s1->GetFileTime() == s2->GetFileTime()) ? false : true;
      }
  };

  typedef hash_set<DictNode*, dict_node_hash>           dict_set_t;
  dict_set_t       m_setDict;

  rc_t loadDictFromFile(IDictIX* pIDictIX, IFileIteratorBase* pIFileIterator, file_id_t file_id);
  void clearDictMap();

  rc_t addDict(const char_t* strCollector, const char_t* strDir, file_id_t, uint64_t);
  DictNode* findDict(const char_t* strCollector, const char_t* strDir, uint64_t);

  IDictIX* getNearDict(IFileIteratorBase* pIFileIterator, const char_t* strCollector, const char_t* strDir, uint64_t);

  rc_t getCollectorDict();

  TC50Dict          m_TC50Dict;

  // parse
private:
  rc_t parseLog(const char_t* strCollector, IDictIX* pIDictIX
    , const char_t**, const char_t*, uint32_t, uint32_t, LogFileNode&);

  AutoRelease<IParseIX*>      m_autoRelIParseIX;

private:
  uint32_t          m_nCountFailed;
  uint32_t          m_timeAlive;

  uint32_t          m_nTransformRows;

private:
  rc_t writeLog(uint32_t, const char_t* strCollector, file_id_t file_id, file_size_t s_pos, uint32_t e_pos, TC50Log* pTC50Log);
private:
  rc_t logDB(ILogWriter::log_lv_t lv, const char_t* from
           , const uint8_t* data, uint32_t dlen
           , const uint8_t* content, uint32_t clen);

private:
  volatile bool_t             m_bRunning;

  IAggregatorBase*            m_pIAggregator;
  static const char_t*        m_strName;
  static const uuid           m_gcUUID;
  DISALLOW_COPY_AND_ASSIGN(ParserTC50);
}; // ParserTC50

#define STR_VERSION           "v1.0.0.1031"

END_NAMESPACE_AGGREGATOR
#endif // PARSER_TC50_H_

