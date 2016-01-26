/*

*/

#ifndef LOG_H_
#define LOG_H_

#include "base.h"
#include "base_export.h"

#include "object.h"
#include "atomic_count.h"
#include "memory_pool.h"
#include "time_util.h"

#include "notify_queue.h"

BEGIN_NAMESPACE_BASE
template<typename _Ta, typename _Tl>
class BASE_EXPORT LogBase : public ILogWriter {

  // log
public:
  void Release() {}

public:
  rc_t Log(log_lv_t lv, IMemoryNode* pIMemoryNode) {

    ASSERT(pIMemoryNode);
    if (lv > m_min_lv) { return RC_E_ACCESS; }

    return log(lv, pIMemoryNode);
  }

  rc_t Log(log_lv_t lv, const uint8_t* pLogData, uint32_t len) {

    ASSERT(pLogData && len);
    if (lv > m_min_lv) { return RC_E_ACCESS; }

    AutoRelease<memory_node_t*> autoMemNode(getMemoryNode(len));
    if (NULL == autoMemNode) { return RC_E_NOMEM; }

    MEMCPY(autoMemNode->data(), autoMemNode->len(), pLogData , len);
    return log(lv, autoMemNode);
  }

  rc_t Log(log_lv_t lv, const char_t* fmt, ...) {

    ASSERT(fmt);
    if (lv > m_min_lv) { return RC_E_ACCESS; }

    const uint32_t buffer_len = 4*1024;
    AutoRelease<memory_node_t*> autoMemNode(getMemoryNode(buffer_len));
    if (NULL == autoMemNode) { return RC_E_NOMEM; }

    va_list arglist;
    va_start(arglist, fmt);
    int len = VSNPRINTF((char_t *)(autoMemNode->data()), autoMemNode->len(), autoMemNode->len(), fmt, arglist);
    va_end(arglist);

    if (len < 0) { return RC_S_FAILED; }

    autoMemNode->Set(len);    
    return log(lv, autoMemNode);
  }

  rc_t Log(log_lv_t lv, const char_t* fmt, va_list arglist) {

    ASSERT(fmt);
    if (lv > m_min_lv) { return RC_E_ACCESS; }

    const uint32_t buffer_len = 4*1024;
    AutoRelease<memory_node_t*> autoMemNode(getMemoryNode(buffer_len));
    if (NULL == autoMemNode) { return RC_E_NOMEM; }

    int len = VSNPRINTF((char_t *)(autoMemNode->data()), autoMemNode->len(), autoMemNode->len(), fmt, arglist);
    if (len < 0) { return RC_S_FAILED; }

    autoMemNode->Set(len);    
    return log(lv, autoMemNode);
  }

  rc_t Commit(ILogWriter* pILogWriter) { return NULL == pILogWriter ? RC_S_NULL_VALUE: commit(pILogWriter); }

  rc_t SetListener(IListener* pIListener) {
    _Ta a(m_l); ignore_result(a);
    return m_queueLogNode.SetListener(pIListener);
  }

  ILogNode* Get() {

    _Ta a(m_l); ignore_result(a);
    if (true == m_queueLogNode.empty()) { return NULL; }

    ILogNode* pILogNode = m_queueLogNode.front();
    m_queueLogNode.pop();

    return pILogNode;
  }

  ILogNode* Peek() { 

    _Ta a(m_l); ignore_result(a);
    if (true == m_queueLogNode.empty()) { return NULL; } 

    ILogNode* pILogNode = m_queueLogNode.front();
    if (NULL == pILogNode) { return NULL; }

    pILogNode->AddRef();
    return pILogNode;
  }

public:
  LogBase(log_lv_t min_lv = LV_TRACE, size_t memSize = 4 * 1024 * 1024, size_t queueSize = 1024)
    : m_min_lv(min_lv)
    , m_mpLog(memSize)
    //, m_queueLogNode(queueSize)
  {
    UNUSED_PARAM(queueSize);
  }

  virtual ~LogBase() {

    while (!m_queueLogNode.empty()) {

      ILogNode* pILogNode = m_queueLogNode.front();
      m_queueLogNode.pop();

      if (NULL == pILogNode) { continue; }
      pILogNode->Release();
    }
  }

  log_lv_t MinLv() const { return m_min_lv; }
  size_t MemSize() const { return m_mpLog.total_size(); }
  void Set(log_lv_t min_lv, size_t memSize, size_t queueSize = 1024) { m_min_lv = min_lv; m_mpLog.resize(memSize); UNUSED_PARAM(queueSize); }

protected:
  rc_t commit(ILogWriter* pILogWriter) {

    ASSERT(pILogWriter);
    _Ta a(m_l); ignore_result(a);

    // move queue
    ILogNode* pILogNode;
    while (NULL !=(pILogNode = pILogWriter->Get())) {

      m_queueLogNode.push(pILogNode);
    }
    return RC_S_OK;
  }

  rc_t log(log_lv_t lv, IMemoryNode* pIMemoryNode) {

    _Ta a(m_l); ignore_result(a);

    AutoRelease<LogNode*> autoRelease(LogNode::CreateInstance(m_mpLog, lv, pIMemoryNode));
    if (NULL == autoRelease) { return RC_E_NOMEM; }

    autoRelease->AddRef();
    m_queueLogNode.push(autoRelease);
    return RC_S_OK;
  }

  typedef MemoryNode<MemoryPool>      memory_node_t;
  memory_node_t* getMemoryNode(uint32_t size) {
    _Ta a(m_l); ignore_result(a);
    return memory_node_t::CreateInstance(m_mpLog, size);
  }

  // memory
protected:
  MemoryPoolNoThreadSafe      m_mpLog;
  friend class LogPoolNode;

  // log node
private:  
  class LogNode : public ILogNode {

    // ILogNode
  public:
    void Release() {

      ASSERT(m_acRef);
      if (0 == --m_acRef) {
        if (m_pIMemoryNode) { m_pIMemoryNode->Release(); }
        m_poolMem.release(this);
      }
    }

  public:
    log_lv_t Type() const { return m_lv; }
    uint32_t Time32() const { return (uint32_t)(micro_time::to_millisecond(m_nTick)); }
    uint64_t Time64() const { return m_nTick; }
    const IMemoryNode* MemNode() const { return m_pIMemoryNode; }

  public:
    static LogNode* CreateInstance(MemoryPool& poolMem, log_lv_t lv, IMemoryNode* pIMemoryNode) {

      if (NULL == pIMemoryNode) { return NULL; }
      uint32_t nodeSize = sizeof(LogNode);
      void* mem = poolMem.get(nodeSize);
      if (NULL == mem) { return NULL; }
      return new(mem) LogNode(poolMem, lv, pIMemoryNode);
    }

    void AddRef() { ++m_acRef; }

  private:
    LogNode(MemoryPool& poolMem, log_lv_t lv, IMemoryNode* pIMemoryNode)
      : m_lv(lv)
      , m_pIMemoryNode(pIMemoryNode)
      , m_poolMem(poolMem)
      , m_acRef(1)
    {
      ASSERT(m_pIMemoryNode);
      m_pIMemoryNode->AddRef();
      m_nTick = micro_time::time();
    }

    ~LogNode() { ASSERT(0); }

  private:
    log_lv_t        m_lv;
    uint64_t        m_nTick; // micro_time
    IMemoryNode*    m_pIMemoryNode;

    MemoryPool&     m_poolMem;
    atomic_count    m_acRef;

    DISALLOW_COPY_AND_ASSIGN(LogNode);
  }; // LogNode

  _Tl               m_l;

  typedef NotifyQueueNoThreadSafe<ILogNode*>      log_node_queue_t;
  log_node_queue_t  m_queueLogNode;

  log_lv_t          m_min_lv;

  DISALLOW_COPY_AND_ASSIGN(LogBase);
}; // LogBase

typedef LogBase<NullClass, NullClass>   LogBaseNoThreadSafe;
END_NAMESPACE_BASE

//////////////////////////////////////////////////////////////////////////
// 
BEGIN_NAMESPACE_BASE

#include "lock.h"
typedef LogBase<AutoLock, Lock>         LogBaseThreadSafe;

END_NAMESPACE_BASE

//////////////////////////////////////////////////////////////////////////
// log pool
#include "pool.h"

BEGIN_NAMESPACE_BASE

typedef struct LogPoolNodeParam {

  ILogWriter*       pLogHost;
  size_t            memSize;
} LogPoolNodeParam_t;

class LogPoolNode;
typedef PoolThreadSafe<LogPoolNode, LogPoolNodeParam>  LogPoolThreadSafe;

//////////////////////////////////////////////////////////////////////////
// no thread safe
class BASE_EXPORT LogPoolNode : public LogBaseNoThreadSafe {

public:
  void Release() {

    if (FALSE == m_bStatus || NULL == m_pLogHost) { return; }

    Commit();
    m_poolLog->release(this);
  }

  rc_t Commit(ILogWriter* pILogWriter = NULL) { UNUSED_PARAM(pILogWriter); return m_pLogHost->Commit(this); }

public:
  LogPoolNode(void* pool, LogPoolNodeParam logPoolNodeParam)
    : m_poolLog(static_cast<LogPoolThreadSafe*>(pool))
    , m_bStatus(FALSE)
    , m_pLogHost(NULL)
  {

    if (RC_S_OK != m_mpLog.resize(logPoolNodeParam.memSize)) { return; }

    m_pLogHost = logPoolNodeParam.pLogHost;
    m_bStatus = TRUE;
  }

  ~LogPoolNode() {

    if (FALSE == m_bStatus) { return; }

    // commit 
    ASSERT(m_pLogHost);
    m_pLogHost->Commit(this);
  }

private:

  bool_t            m_bStatus;
  LogPoolThreadSafe*m_poolLog;
  ILogWriter*       m_pLogHost;

  DISALLOW_COPY_AND_ASSIGN(LogPoolNode);
}; //LogPoolNode


END_NAMESPACE_BASE
#endif // LOG_H_
