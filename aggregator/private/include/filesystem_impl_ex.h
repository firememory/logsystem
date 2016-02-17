/*

*/

#ifndef FILESYSTEM_IMPL_H_
#define FILESYSTEM_IMPL_H_

// logsystem
#include "filesystem.h"
//

// base
#include "atomic_count.h"
#include "hash_tables.h"
#include "lock.h"
//#include "pool.h"
//#include "file_util.h"
#include "time_util.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////
USING_BASE(DayTime);
USING_BASE(Lock);
USING_BASE(AutoLock);
USING_BASE(atomic_count);
USING_BASE(hash_map);
/*
USING_BASE(OpenFile);
USING_BASE(ReadFile);
USING_BASE(WriteFile);
USING_BASE(strAttrOpenRead);
USING_BASE(strAttrOpenWrite);

USING_BASE(FlushFile);
USING_BASE(MakeDirectory);
USING_BASE(PlatformFileInfo);
USING_BASE(GetPlatformFileInfo);
*/
class FileSystemImpl : public IFileSystem {

  // IResource
public:
  const rc_t GetName(char_t*, uint32_t*) const;
  const rc_t GetStatus() const;

  const rc_t GetResourceSize(uint64_t*, uint64_t*) const;
  const rc_t GetLastErr(uint32_t*, char_t*, uint32_t*) const;
  const rc_t GetInfo(char_t*, uint32_t*) const;
  const rc_t GetLastAliveTime(uint32_t*) const;

  // IFileSystem
  // public:
  //   void Release();

public:
  IMemoryNode* GetData(file_id_t file_id, file_size_t* pos, file_size_t size);

  //IFileContainer* GetFileContainer(const char_t*);
  IFileNode* GetFileNode(file_id_t);

public:
  const IFileNode* AddFile(file_id_t file_id, const char_t* collector
    , const char_t* strDir, const char_t* strName, uint64_t create_time);
  rc_t SetData(file_size_t*, file_id_t file_id, IMemoryNode* memNode, file_size_t pos, file_size_t len);

  const IFileNode* AddFile(const file_id_t& file_id, const char_t* collector, const char_t* strLocalPath
    , const char_t* strDir, const char_t* strName, uint64_t create_time, file_size_t size
    , bool_t bFinish, const char_t* checksum);
  //rc_t AddLogType(const char_t*);

  rc_t FileOver(file_id_t file_id);
  void CloseAllFile();

  void SetRootDir(const char_t* strRootPath);

public:
  IMemoryNode* GetMemory(uint32_t /*size*/) { return m_cacheFileBuffer.GetMemory(); }

public:
  void TickProc();

  DEFINE_REF_COUNT_RELEASE_DEL_SELF
private:
  FileSystemImpl(size_t max_mem);
  ~FileSystemImpl();

public:
  static FileSystemImpl* CreateInstanceEx(size_t max_mem);

private:
  char_t          m_strRootPath[kMAX_PATH + 1];

private:
  //////////////////////////////////////////////////////////////////////////
  class FileNodeImpl : public IFileNode {
  public:
    void Release() { ASSERT(m_acRefCount); --m_acRefCount; }
    void AddRef() { ASSERT(kuint32max > m_acRefCount); ++m_acRefCount; }

  public:
    file_id_t ID() const { return m_id; }
    file_size_t Size() const { return max(m_size, m_nNFPos); }
    STATUS_e Status() const { return m_status; }
    const char_t* CheckSum() const { return m_checksum; }
    const char_t* RemotePath() const { return m_strLocalPath + m_nRemotePathPos; } // RemotePath
    const char_t* LocalPath() const { return m_strLocalPath; } // RemotePath
    const char_t* Collector() const { return m_collector; }
    const uint64_t CreateTime() const { return m_create_time; }

  public:
    file_size_t GetNFPos() const { return m_nNFPos; }
    void SetID(file_id_t id) { m_id = id; }
    void SetSize(file_size_t ) {}

  public:
    FileNodeImpl(const char_t* strLocalEx, file_id_t file_id, const char_t* collector
      , const char_t* strDir, const char_t* strName, uint64_t create_time)
      : m_acRefCount(0)

      , m_lockFileData()
      , m_nFileStartPos(0)

      , m_id(file_id)
      , m_create_time(create_time)
      , m_size(0)
      , m_bOver(FALSE)
      , m_status(PROCESS)
      , m_nNFPos(0)
    {
      ASSERT(strLocalEx);
      ASSERT(strDir);
      ASSERT(strName);
      ASSERT(collector);

      STRCPY(m_collector, kMAX_NAME_LEN, collector);

      size_t len = SNPRINTF(m_strLocalPath, kMAX_PATH, _STR("%s/%s/%s/%s"), strLocalEx, collector, strDir, strName);

      // driver 
      char_t* strFind = STRCHR(m_strLocalPath, len, _CHAR(':'));
      if (strFind) { (*strFind) = _CHAR('_'); }
      m_nRemotePathPos = len - (STRLEN(strDir) + STRLEN(strName) + 1);

      BZERO_ARR(m_checksum);
      m_pFileData[0] = NULL;
      m_pFileData[1] = NULL;

      ReSetTick();
    }

    FileNodeImpl(const file_id_t& file_id, const char_t* collector, const char_t* strLocalPath
      , /*const char_t* strDir, const char_t* strName, */uint64_t create_time
      , file_size_t size, bool_t bFinish, const char_t* checksum)
      : m_acRefCount(0)

      , m_lockFileData()
      , m_nFileStartPos(size)

      , m_id(file_id)
      , m_create_time(create_time)
      , m_size(size)
      , m_bOver(FALSE)
      , m_status(FINISH)
      , m_nRemotePathPos(0)
      , m_nNFPos(size)
    {
      ASSERT(collector);
      ASSERT(strLocalPath);

      m_status = bFinish ? FINISH : PROCESS;
      m_bOver = m_status == FINISH ? TRUE : FALSE;

      STRCPY(m_collector, kMAX_NAME_LEN, collector);
      STRCPY(m_strLocalPath, kMAX_PATH, strLocalPath);

      // driver 
      char_t* strFind = STRSTR(m_strLocalPath, sizeof(m_strLocalPath), collector);
      if (strFind) { m_nRemotePathPos = strFind - m_strLocalPath + STRLEN(collector) + 1; }

      BZERO_ARR(m_checksum);
      if (checksum) { STRCPY(m_checksum, 32, checksum); }

      m_pFileData[0] = NULL;
      m_pFileData[1] = NULL;

      ReSetTick();
    }

    ~FileNodeImpl() { 
      ASSERT(0 == m_acRefCount); 
      if (m_pFileData[0]) { m_pFileData[0]->Release(); m_pFileData[0] = NULL; }
      if (m_pFileData[1]) { m_pFileData[1]->Release(); m_pFileData[1] = NULL; }
    }

  public:
    IMemoryNode* GetData(file_size_t* pos) {

      if (NULL == pos) { return NULL; }
      m_nTimeLastRead = DayTime::now();

      AutoLock autoLock(m_lockFileData);

      if ((*pos) < m_nFileStartPos) {

        if (m_pFileData[0]) { m_pFileData[0]->Release(); m_pFileData[0] = NULL; }
        if (m_pFileData[1]) { m_pFileData[1]->Release(); m_pFileData[1] = NULL; }

        m_nNFPos        = (*pos);
        m_size          = m_nNFPos;
        m_nFileStartPos = m_nNFPos;
        return NULL;
      }

      if ((*pos) - m_nFileStartPos < CacheFileData::kFILE_NODE_SIZE) {

        // one
        if (m_pFileData[0]) {
          m_pFileData[0]->AddRef();
          (*pos) = m_nFileStartPos;
        }
        else { m_nNFPos = (*pos); }
        return m_pFileData[0];
      }
      else if ((*pos) - m_nFileStartPos < 2 * CacheFileData::kFILE_NODE_SIZE) {

        // two
        if (m_pFileData[1]) {
          m_pFileData[1]->AddRef();
          (*pos) = m_nFileStartPos + CacheFileData::kFILE_NODE_SIZE;
        }
        else { m_nNFPos = (*pos); }
        return m_pFileData[1];
      }
      else {
        // three
        m_nNFPos = (*pos);
        return NULL;
      }
    }

    rc_t SetData(IMemoryNode* memNode, file_size_t pos, uint32_t len) {

      if (NULL == memNode) { return RC_S_NULL_VALUE; }
      m_nTimeLastRead = DayTime::now();

      {
        CacheFileData::FileDataMemNode* pFileDataMemNode = (CacheFileData::FileDataMemNode*)(memNode);
        pFileDataMemNode->SetLen(len);
      }

      AutoLock autoLock(m_lockFileData);
      if (pos < m_nFileStartPos) {

        if (m_pFileData[0]) { m_pFileData[0]->Release(); m_pFileData[0] = NULL; }
        if (m_pFileData[1]) { m_pFileData[1]->Release(); m_pFileData[1] = NULL; }

        memNode->AddRef();
        m_pFileData[0] = memNode;
        m_nFileStartPos = pos;

        m_size = pos + len;

        CacheFileData::FileDataMemNode* pFileDataMemNode = (CacheFileData::FileDataMemNode*)(m_pFileData[0]);
        ASSERT(pFileDataMemNode);
        pFileDataMemNode->SetLen(len);
        return RC_S_OK;
      }

      size_t nRealPos = (size_t)(pos - m_nFileStartPos);
      if (nRealPos < CacheFileData::kFILE_NODE_SIZE) {

        if (m_pFileData[1]) { m_pFileData[1]->Release(); m_pFileData[1] = NULL; }

        // one
        if (m_pFileData[0]) {

          CacheFileData::FileDataMemNode* pFileDataMemNode = (CacheFileData::FileDataMemNode*)(m_pFileData[0]);
          ASSERT(pFileDataMemNode);

          size_t nFreeSize = CacheFileData::kFILE_NODE_SIZE - nRealPos;
          //size_t nCopySize = len % nFreeSize;
          size_t nCopySize = len > nFreeSize ? nFreeSize : len;
          // copy data
          //MEMCPY(pFileDataMemNode->data() + nRealPos, nFreeSize, memNode->data(), nCopySize);
          MEMCPY(pFileDataMemNode->data() + (CacheFileData::kFILE_NODE_SIZE - nFreeSize), nFreeSize, memNode->data(), nCopySize);
          m_size += nCopySize;

          pFileDataMemNode->SetLen(pFileDataMemNode->len() + nCopySize);
          return RC_S_OK;
        }
        
        //
        memNode->AddRef();
        m_pFileData[0] = memNode;
        m_nFileStartPos = pos;
        m_size = pos + len;

        CacheFileData::FileDataMemNode* pFileDataMemNode = (CacheFileData::FileDataMemNode*)(m_pFileData[0]);
        ASSERT(pFileDataMemNode);
        pFileDataMemNode->SetLen(len);
        return RC_S_OK;
      }
      else if (nRealPos < 2 * CacheFileData::kFILE_NODE_SIZE) {

        // two
        if (m_pFileData[1]) {
          CacheFileData::FileDataMemNode* pFileDataMemNode = (CacheFileData::FileDataMemNode*)(m_pFileData[1]);
          ASSERT(pFileDataMemNode);

          size_t nFreeSize = 2 * CacheFileData::kFILE_NODE_SIZE - nRealPos;
          //size_t nCopySize = len % nFreeSize;
          size_t nCopySize = len > nFreeSize ? nFreeSize : len;
          // copy data
          //MEMCPY(pFileDataMemNode->data() + nRealPos, nFreeSize, memNode->data(), nCopySize);
          MEMCPY(pFileDataMemNode->data() + (CacheFileData::kFILE_NODE_SIZE - nFreeSize), nFreeSize, memNode->data(), nCopySize);
          m_size += nCopySize;

          pFileDataMemNode->SetLen(pFileDataMemNode->len() + nCopySize);
          return RC_S_OK;
        }

        memNode->AddRef();
        m_pFileData[1] = memNode;
        m_size = pos + len;

        CacheFileData::FileDataMemNode* pFileDataMemNode = (CacheFileData::FileDataMemNode*)(m_pFileData[1]);
        ASSERT(pFileDataMemNode);
        pFileDataMemNode->SetLen(len);

        return RC_S_OK;
      }
      else {
        // three
        if (m_pFileData[0]) {
          m_pFileData[0]->Release();
          m_pFileData[0] = NULL;
          m_nFileStartPos += CacheFileData::kFILE_NODE_SIZE;
        }
        else { m_nFileStartPos = pos; }

        memNode->AddRef();
        if (m_pFileData[1]) { m_pFileData[0] = m_pFileData[1]; m_pFileData[1] = memNode;}
        else { m_pFileData[0] = memNode; }

        m_size = pos + len;
        return RC_S_OK;
      }
    }

    rc_t OverFile() {
      AutoLock autoLock(m_lockFileData);
      if (m_pFileData[0]) { m_pFileData[0]->Release(); m_pFileData[0] = NULL; }
      if (m_pFileData[1]) { m_pFileData[1]->Release(); m_pFileData[1] = NULL; }
      return RC_S_OK;
    }

    void TickProc() {
      uint32_t now = DayTime::now();
      if (now - m_nTimeLastRead > kCloseFileTimeOut) { OverFile(); }
    }

    void ReSetTick() {
      m_nTimeLastRead  = DayTime::now();
      m_nTimeLastWrite = DayTime::now();
    }

  private:
    uint32_t      m_nTimeLastRead;
    uint32_t      m_nTimeLastWrite;

    Lock          m_lockFileData;
    IMemoryNode*  m_pFileData[2];
    file_size_t   m_nFileStartPos;

    atomic_count  m_acRefCount;

    file_id_t     m_id;
    char_t        m_strLocalPath[kMAX_PATH + 1];
    size_t        m_nRemotePathPos;
    char_t        m_collector[kMAX_NAME_LEN + 1];
    file_size_t   m_size;
    bool_t        m_bOver;

    STATUS_e      m_status;
    char_t        m_checksum[32 + 1];

    uint64_t      m_create_time;

    file_size_t   m_nNFPos;

    DISALLOW_COPY_AND_ASSIGN(FileNodeImpl);
  }; // FileNodeImpl

  typedef hash_map<file_id_t, FileNodeImpl*>   filenode_map_t;
  filenode_map_t    m_mapFileNode;

  FileNodeImpl* findFileNode(file_id_t);
  rc_t updateFileNode(FileNodeImpl*, IMemoryNode* memNode, file_size_t pos, file_size_t len);

  //////////////////////////////////////////////////////////////////////////
  // file data buffer
  class CacheFileData {
  public:
    CacheFileData(size_t max_mem)
      : m_nCacheSize(max_mem)
      , m_nAllocCacheSize(0)
      , m_nCountGetMemFailed(0)
      , m_lockCache()
    { initCacheData(); }

    ~CacheFileData() { clearCacheData(); }

  public:
    //void Set(size_t max_mem, uint32_t cache_size, uint32_t cache_count_file_block);
    //rc_t Put(file_id_t file_id, file_size_t pos, IMemoryNode* memNode);
    //rc_t Get(IMemoryNode** memNode, file_id_t file_id, file_size_t* pos);
    IMemoryNode* GetMemory(/*uint32_t size*/) {
      //if (size > kFILE_NODE_SIZE) { return NULL; }

      AutoLock autoLock(m_lockCache);
      if (m_queueFreeMemory.empty()) { 
        initCacheData();
        if (m_queueFreeMemory.empty()) { return NULL; }
      }

      IMemoryNode* pIMemoryNode = m_queueFreeMemory.front();
      ASSERT(pIMemoryNode);
      pIMemoryNode->AddRef();

      m_queueFreeMemory.pop();
      return pIMemoryNode;
    }

    void ReleaseMemory(IMemoryNode* pIMemoryNode) {
      AutoLock autoLock(m_lockCache);
      m_queueFreeMemory.push(pIMemoryNode);
    }

  public:
    inline rc_t GetCacheCount(uint32_t* file_count, uint32_t* file_block_count) const {

      if (NULL == file_count || NULL == file_block_count) { return RC_S_NULL_VALUE; }
      /*
      (*file_count) = (*file_block_count) = 0;

      CacheFile_t::const_iterator it_cache, end;
      for (it_cache = m_cacheFile.begin(), end = m_cacheFile.end(); it_cache != end; ++it_cache) {

        const CacheFileData_t* pCacheFileData = it_cache->second;
        if (NULL == pCacheFileData) { continue; }

        (*file_block_count) += (uint32_t)pCacheFileData->size();
        ++(*file_count);
      }
      */
      return RC_S_OK;
    }

    inline rc_t GetMemorySize(size_t* total_size, size_t* free_size) const {

      if (NULL == total_size || NULL == free_size) { return RC_S_NULL_VALUE; }

      (*total_size) = m_nAllocCacheSize;
      (*free_size) = 0;
      return RC_S_OK;
    }

    inline uint32_t GetCountGetMemFailed() const { return m_nCountGetMemFailed; }

  public:
    enum { kMIN_ALLOC_COUNT = 128, kFILE_NODE_SIZE = 256 * 1024, kPRE_BUF_SIZE = 1024 * 1024 };

  private:
    size_t          m_nCacheSize;
    size_t          m_nAllocCacheSize;
    uint32_t        m_nCountGetMemFailed;

  private:
    Lock            m_lockCache;

  private:
    class FileDataMemNode : public IMemoryNode {
      // IMemoryNode
    public:
      void Release() {
        ASSERT(m_acRef);
        if (0 == --m_acRef) { m_len = kFILE_NODE_SIZE; m_cache.ReleaseMemory(this); }
      }

      void AddRef() { ++m_acRef; }

    public:
      uint8_t* data() const { return m_data; }
      uint32_t len() const { ASSERT(kFILE_NODE_SIZE >= m_len); return m_len; }

    public:
      void SetLen(uint32_t len) {
        m_len = kFILE_NODE_SIZE >= len ? len : kFILE_NODE_SIZE;
        /*ASSERT(kFILE_NODE_SIZE >= len); m_len = len; */
      }

    private:
      atomic_count  m_acRef;

    public:
      FileDataMemNode(CacheFileData& cache, uint8_t* pMem)
        : m_acRef(0)
        , m_cache(cache)
        , m_data(pMem)
        , m_len(kFILE_NODE_SIZE)
      {}

    private:
      CacheFileData&          m_cache;
      uint8_t*                m_data;
      uint32_t                m_len;

      DISALLOW_COPY_AND_ASSIGN(FileDataMemNode);
    };

    // drop this
    friend class FileNodeImpl;

    // mem
  private:
    typedef std::list<uint8_t*>  mem_list_t;
    mem_list_t      m_listMemory;

    typedef std::list<FileDataMemNode*>  memnode_list_t;
    memnode_list_t  m_listMemoryNode;

    typedef std::queue<IMemoryNode*>  mem_queue_t;
    mem_queue_t     m_queueFreeMemory;

  private:
    void initCacheData() {

      size_t nCount = m_nCacheSize - m_nAllocCacheSize;
      if (nCount > kMIN_ALLOC_COUNT) { nCount = kMIN_ALLOC_COUNT; }

      for (size_t idx = 0; idx < nCount; ++idx) {
        uint8_t* pMemory = (uint8_t*)(malloc(kPRE_BUF_SIZE));
        if (NULL == pMemory) { continue; }
        ++m_nAllocCacheSize;
        // push
        for (size_t nPushIdx = 0; nPushIdx < kPRE_BUF_SIZE / kFILE_NODE_SIZE; ++nPushIdx) {

          FileDataMemNode* pFileDataMemNode = new FileDataMemNode(*this, pMemory + nPushIdx * kFILE_NODE_SIZE);
          if (NULL == pFileDataMemNode) { continue; }

          m_listMemoryNode.push_back(pFileDataMemNode);
          m_queueFreeMemory.push(pFileDataMemNode);
        }
      }
    }

    void clearCacheData() {

      // node
      {
        memnode_list_t::iterator it_list, end;
        for (it_list = m_listMemoryNode.begin(), end = m_listMemoryNode.end(); it_list != end; ++it_list) {
          delete *it_list;
        }
      }

      // memory
      {
        mem_list_t::iterator it_list, end;
        for (it_list = m_listMemory.begin(), end = m_listMemory.end(); it_list != end; ++it_list) {
          free(*it_list);
        }
      }
    }

    DISALLOW_COPY_AND_ASSIGN(CacheFileData);
  }; // CacheFileData

  CacheFileData   m_cacheFileBuffer;

private:
  static const char_t*        m_strName;
  uint32_t                    m_timeAlive;

  DISALLOW_COPY_AND_ASSIGN(FileSystemImpl);
}; // FileSystemImpl

#define STR_VERSION           "v1.0.0.910"

END_NAMESPACE_AGGREGATOR

#endif // FILESYSTEM_IMPL_H_
