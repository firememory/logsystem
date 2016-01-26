/*

*/

#ifndef CACHE_FILEDATA_H_
#define CACHE_FILEDATA_H_


#include "aggregator_base.h"

#include "object.h"
#include "lock.h"
#include "memory_pool.h"
#include "mru_cache.h"

BEGIN_NAMESPACE_AGGREGATOR

USING_BASE(MemoryPoolThreadSafe);

USING_BASE(MRUCache);
USING_BASE(HashingMRUCache);
USING_BASE(MRUCachePointerDeletor);
USING_BASE(Lock);

//////////////////////////////////////////////////////////////////////////
class CacheFileData {
public:
  CacheFileData(size_t max_mem, uint32_t cache_count, uint32_t cache_count_file_block);
  ~CacheFileData();

public:
  void Set(size_t max_mem, uint32_t cache_size, uint32_t cache_count_file_block);
  rc_t Put(file_id_t file_id, file_size_t pos, IMemoryNode* memNode);
  rc_t Get(IMemoryNode** memNode, file_id_t file_id, file_size_t* pos);
  IMemoryNode* GetMemory(uint32_t size);

public:
  inline rc_t GetCacheCount(uint32_t* file_count, uint32_t* file_block_count) const {

    if (NULL == file_count || NULL == file_block_count) { return RC_S_NULL_VALUE; }

    (*file_count) = (*file_block_count) = 0;

    CacheFile_t::const_iterator it_cache, end;
    for (it_cache = m_cacheFile.begin(), end = m_cacheFile.end(); it_cache != end; ++it_cache) {

      const CacheFileData_t* pCacheFileData = it_cache->second;
      if (NULL == pCacheFileData) { continue; }

      (*file_block_count) += (uint32_t)pCacheFileData->size();
      ++(*file_count);
    }

    return RC_S_OK;
  }

  inline rc_t GetMemorySize(size_t* total_size, size_t* free_size) const {

    if (NULL == total_size || NULL == free_size) { return RC_S_NULL_VALUE; }

    MemoryPoolThreadSafe& poolMemory = *(MemoryPoolThreadSafe*)(this);
    (*total_size) = poolMemory.total_size();
    (*free_size) = poolMemory.free_size();
    return RC_S_OK;
  }

  inline uint32_t GetCountGetMemFailed() const { return m_nCountGetMemFailed; }
private:
  struct FileDataItem {
    file_size_t pos;
    IMemoryNode* memNode;
  };

  MemoryPoolThreadSafe  m_poolFileDataMemory;
  size_t                m_nPoolSize;

private:
  uint32_t              m_nCountGetMemFailed;

private:

  template<class PayloadType>
  class MRUCacheFileDataItemDeletor {
  public:
    void operator()(PayloadType& payload) {
      payload.memNode->Release();
    }
  };

  static const uint32_t kMAX_CACHE_FILE_SIZE = 256;
  typedef MRUCache<file_size_t, FileDataItem, MRUCacheFileDataItemDeletor<FileDataItem> > CacheFileData_t;
  
  static const uint32_t kMAX_CACHE_SIZE = 1024;
  typedef HashingMRUCache<file_id_t, CacheFileData_t*, MRUCachePointerDeletor<CacheFileData_t*> > CacheFile_t;
  CacheFile_t         m_cacheFile;
  Lock                m_lockCache;

  void clearCacheData();
  uint32_t m_cache_count_file_block;

  DISALLOW_COPY_AND_ASSIGN(CacheFileData);
}; // CacheFileData


END_NAMESPACE_AGGREGATOR

#endif // CACHE_FILEDATA_H_
