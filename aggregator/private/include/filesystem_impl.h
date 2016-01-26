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
#include "pool.h"
#include "file_util.h"
#include "cache_filedata.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////
USING_BASE(DayTime);
USING_BASE(Lock);
USING_BASE(AutoLock);
USING_BASE(atomic_count);
USING_BASE(hash_map);

USING_BASE(OpenFile);
USING_BASE(ReadFile);
USING_BASE(WriteFile);
USING_BASE(strAttrOpenRead);
USING_BASE(strAttrOpenWrite);

USING_BASE(FlushFile);
USING_BASE(MakeDirectory);
USING_BASE(PlatformFileInfo);
USING_BASE(GetPlatformFileInfo);

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
  rc_t SetData(file_size_t*, file_id_t file_id, IMemoryNode* memNode, file_size_t pos);

  const IFileNode* AddFile(const file_id_t& file_id, const char_t* collector, const char_t* strLocalPath
    , const char_t* strDir, const char_t* strName, uint64_t create_time, file_size_t size
    , bool_t bFinish, const char_t* checksum);
  //rc_t AddLogType(const char_t*);

  rc_t FileOver(file_id_t file_id);
  void CloseAllFile();

  void SetRootDir(const char_t* strRootPath);

public:
  IMemoryNode* GetMemory(uint32_t size) { return m_cacheFileBuffer.GetMemory(size); }

public:
  void TickProc();

  DEFINE_REF_COUNT_RELEASE_DEL_SELF
private:
  FileSystemImpl(size_t max_mem, uint32_t cache_count, uint32_t cache_count_file_block);
  ~FileSystemImpl();

public:
  static FileSystemImpl* CreateInstanceEx(size_t max_mem, uint32_t cache_count, uint32_t cache_count_file_block);

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
    file_size_t Size() const { return m_size; }
    STATUS_e Status() const { return m_status; }
    const char_t* CheckSum() const { return m_checksum; }
    const char_t* RemotePath() const { return m_strLocalPath + m_nRemotePathPos; } // RemotePath
    const char_t* LocalPath() const { return m_strLocalPath; } // RemotePath
    const char_t* Collector() const { return m_collector; }
    const uint64_t CreateTime() const { return m_create_time; }

  public:
    void SetID(file_id_t id) { m_id = id; }
    void SetSize(file_size_t ) { }

  public:
    FileNodeImpl(const char_t* strLocalEx, file_id_t file_id, const char_t* collector
      , const char_t* strDir, const char_t* strName, uint64_t create_time)
      : m_acRefCount(0)
      , m_id(file_id)
      , m_create_time(create_time)
      , m_size(0)
      , m_bOver(FALSE)
      , m_status(PROCESS)
      , m_autoRelFileWrite(NULL)
      , m_autoRelFileRead(NULL)
    {
      ASSERT(strLocalEx);
      ASSERT(strDir);
      ASSERT(strName);
      ASSERT(collector);

      STRCPY(m_collector, kMAX_NAME_LEN, collector);

      size_t len = SNPRINTF(m_strLocalPath, kMAX_PATH, _STR("%s/%s/%s/%s"), strLocalEx, collector, strDir, strName);

      // driver 
      char_t* strFind = STRCHR(m_strLocalPath, _CHAR(':'));
      if (strFind) { (*strFind) = _CHAR('_'); }
      m_nRemotePathPos = len - (STRLEN(strDir) + STRLEN(strName) + 1);

      BZERO_ARR(m_checksum);

      ReSetTick();
    }

    FileNodeImpl(const file_id_t& file_id, const char_t* collector, const char_t* strLocalPath
      , /*const char_t* strDir, const char_t* strName, */uint64_t create_time
      , file_size_t size, bool_t bFinish, const char_t* checksum)
      : m_acRefCount(0)
      , m_id(file_id)
      , m_create_time(create_time)
      //, m_size(size)
      , m_bOver(FALSE)
      , m_status(FINISH)
      , m_autoRelFileWrite(NULL)
      , m_autoRelFileRead(NULL)
      , m_nRemotePathPos(0)
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

      // get rael file size
      m_size = getRealSize(size);

      ReSetTick();
    }

    ~FileNodeImpl() { ASSERT(0 == m_acRefCount); }

  private:    
    void openFileRead() {

      if (NULL == m_autoRelFileRead) {
        m_autoRelFileRead.Set(OpenFile(m_strLocalPath, strAttrOpenRead));
      }
    }

    void openFile() {

      if (NULL == m_autoRelFileWrite) {
        if (FALSE == MakeDirectory(m_strLocalPath)) { return ; }
        m_autoRelFileWrite.Set(OpenFile(m_strLocalPath, strAttrOpenWrite));
      }

            // file already Exists
            // backup
//             char_t strBackUpPath[kMAX_PATH + 1] = {0};
//             SNPRINTF(strBackUpPath, kMAX_PATH, "%s.bc", m_strLocalPath);
//             ::CloseFile(m_file_handle);
//             m_file_handle = NULL;
// 
//             ::Copy(m_strLocalPath, strBackUpPath);
//             ::Move(m_strLocalPath, m_strLocalTempPath);
    }

  public:
    rc_t GetData(uint8_t* data, file_size_t* read_size, file_size_t pos) {

      m_nTimeLastRead = DayTime::now();

      AutoLock  autoLock(m_lockFile);
      openFileRead();
      if (NULL == m_autoRelFileRead) { return RC_S_FAILED;}

      return ReadFile(data, read_size, m_autoRelFileRead, pos);
    }

    rc_t SetData(IMemoryNode* memNode, file_size_t pos) {

      m_nTimeLastWrite = DayTime::now();

      file_size_t write_size = memNode->len();
      openFile();
      if (NULL == m_autoRelFileWrite) { return RC_S_FAILED;}

      rc_t rc = WriteFile(m_autoRelFileWrite, pos, memNode->data(), &write_size);
      if (RC_S_OK == rc) { 
        FlushFile(m_autoRelFileWrite);
        m_size = pos + write_size;
      }
      return rc;
    }

    rc_t OverFile() {

      // close file handle
      m_autoRelFileWrite.Release();
      // rename
//       if (FALSE == ::Move(m_strLocalTempPath, m_strLocalPath)) {
//         // move failed
//       }
      m_bOver = TRUE;
      return RC_S_OK;
    }

    void TickProc() {

      uint32_t now = DayTime::now();
      if (now - m_nTimeLastRead > kCloseFileTimeOut) { m_autoRelFileRead.Release(); m_nTimeLastRead = now; }
      if (now - m_nTimeLastWrite > kCloseFileTimeOut) { m_autoRelFileWrite.Release(); m_nTimeLastWrite = now; }
    }

    void CloseFile() {

      m_autoRelFileRead.Release();
      m_autoRelFileWrite.Release();
    }

  private:

    file_size_t getRealSize(file_size_t size) {

      openFileRead();
      if (NULL == m_autoRelFileRead) { return size;}

      PlatformFileInfo fileInfo; 
      if (FALSE == GetPlatformFileInfo(m_autoRelFileRead, &fileInfo)) { return size; }
      return fileInfo.size < size ? fileInfo.size : size;
    }

    void ReSetTick() {
      m_nTimeLastRead  = DayTime::now();
      m_nTimeLastWrite = DayTime::now();
    }

  private:
    uint32_t        m_nTimeLastRead;
    uint32_t        m_nTimeLastWrite;
    // read file lock
    Lock            m_lockFile;
    AutoReleaseFile m_autoRelFileRead;

    // write file
    AutoReleaseFile m_autoRelFileWrite;

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

    DISALLOW_COPY_AND_ASSIGN(FileNodeImpl);
  }; // FileNodeImpl

  typedef hash_map<file_id_t, FileNodeImpl*>   filenode_map_t;
  filenode_map_t    m_mapFileNode;

  FileNodeImpl* findFileNode(file_id_t);
  rc_t updateFileNode(FileNodeImpl*, IMemoryNode* memNode, file_size_t pos);

  //////////////////////////////////////////////////////////////////////////
  // file data buffer
  CacheFileData   m_cacheFileBuffer;

private:
  static const char_t*        m_strName;
  uint32_t                    m_timeAlive;

  DISALLOW_COPY_AND_ASSIGN(FileSystemImpl);
}; // FileSystemImpl

#define STR_VERSION           "v1.0.0.910"

END_NAMESPACE_AGGREGATOR

#endif // FILESYSTEM_IMPL_H_
