/*

*/

#include "cache_filedata.h"

BEGIN_NAMESPACE_AGGREGATOR

USING_BASE(AutoLock);
USING_BASE(MemoryNode);

//////////////////////////////////////////////////////////////////////////
CacheFileData::CacheFileData(size_t max_mem, uint32_t cache_count, uint32_t cache_count_file_block)
  : m_poolFileDataMemory(max_mem)
  , m_nPoolSize(max_mem)
  , m_cacheFile(cache_count)
  , m_cache_count_file_block(cache_count_file_block)
  , m_nCountGetMemFailed(0)
{}

CacheFileData::~CacheFileData() {

  clearCacheData();
}


//////////////////////////////////////////////////////////////////////////
void CacheFileData::Set(size_t max_mem, uint32_t cache_count, uint32_t cache_count_file_block) {

  m_nPoolSize = max_mem;
  m_poolFileDataMemory.resize(m_nPoolSize);
  m_cacheFile.ShrinkToSize(cache_count);
  m_cache_count_file_block = cache_count_file_block;
}

rc_t CacheFileData::Put(file_id_t file_id, file_size_t pos, IMemoryNode* memNode) {

  ASSERT(memNode);

  AutoLock autoLock(m_lockCache);
  // 
  CacheFile_t::iterator it_cache;
  it_cache = m_cacheFile.Peek(file_id);

  CacheFileData_t* pCacheFileData;
  if (it_cache == m_cacheFile.end()) {
    // add
    pCacheFileData = new CacheFileData_t(m_cache_count_file_block);
    if (NULL == pCacheFileData) { return RC_E_NOMEM; }

    memNode->AddRef();
    FileDataItem item;
    item.pos = pos;
    item.memNode = memNode;
    pCacheFileData->Put(pos, item);

    m_cacheFile.Put(file_id, pCacheFileData);
  }
  else {
    // update
    pCacheFileData = it_cache->second;
    if (NULL == pCacheFileData) { return RC_S_NULL_VALUE; }

    CacheFileData_t::iterator it_cache_file = pCacheFileData->Peek(pos);
    if (it_cache_file == pCacheFileData->end()) {

      memNode->AddRef();
      FileDataItem item;
      item.pos = pos;
      item.memNode = memNode;

      pCacheFileData->Put(pos, item);
    }
    else {
      FileDataItem& old_item = it_cache_file->second;
      old_item.memNode->Release();
      memNode->AddRef();
      old_item.memNode = memNode;
    }
  }

  return RC_S_OK;
}

rc_t CacheFileData::Get(IMemoryNode** memNode, file_id_t file_id, file_size_t* pos) {

  ASSERT(memNode);

  AutoLock autoLock(m_lockCache);
  CacheFile_t::iterator it_cache = m_cacheFile.Get(file_id);
  if (it_cache == m_cacheFile.end()) { return RC_S_NOTFOUND; }
  
  CacheFileData_t* pCacheFileData = it_cache->second;
  if (NULL == pCacheFileData) { return RC_S_NOTFOUND; }

//   CacheFileData_t::iterator it_cache_file = pCacheFileData->Get(pos);
//   if (it_cache_file == pCacheFileData->end()) { return RC_S_NOTFOUND; }

  CacheFileData_t::iterator it_cache_file = pCacheFileData->GetLowerBound((*pos));
  if (it_cache_file == pCacheFileData->end()) {

    // get next pos
    it_cache_file = pCacheFileData->GetUpperBound((*pos));
    if (it_cache_file == pCacheFileData->end()) { return RC_S_NOTFOUND; }
    return RC_E_INDEX;
  }
/*
  if (it_cache_file->second.pos >  (*pos)) {
    if (--it_cache_file == pCacheFileData->end()) { 
      (*pos) = it_cache_file->second.pos;
      return RC_S_FAILED;
    }
  }
*/
  const FileDataItem& item = it_cache_file->second;
  ASSERT(item.memNode);

  // 
  if (item.pos + item.memNode->len() < (*pos)) { return RC_S_NOTFOUND; }

  item.memNode->AddRef();
  *memNode = item.memNode;

  (*pos) = item.pos;
  return RC_S_OK;
}

IMemoryNode* CacheFileData::GetMemory(uint32_t size) {

  if (m_poolFileDataMemory.total_size() < size) { return NULL; }

  typedef MemoryNode<MemoryPoolThreadSafe>      memory_node_t;
  
  memory_node_t* node;
  while (NULL == (node = memory_node_t::CreateInstance(m_poolFileDataMemory, size))) {

    ++m_nCountGetMemFailed;

    AutoLock autoLock(m_lockCache);

    // 
    CacheFile_t::iterator it_cache, end;
    for (it_cache = m_cacheFile.begin(), end = m_cacheFile.end(); it_cache != end; ++it_cache) {

      CacheFileData_t* pCacheFileData = it_cache->second;
      if (NULL == pCacheFileData) { continue; }

      size_t nBlockCacheSize = pCacheFileData->size();
      if (nBlockCacheSize) { pCacheFileData->ShrinkToSize(nBlockCacheSize - 1); }
    }
    /*
    uint32_t nCacheSize = (uint32_t)m_cacheFile.size();
    if (2 <= nCacheSize) {
      m_cacheFile.ShrinkToSize(nCacheSize - 1);
    }
    else {

      if (1 == nCacheSize) {
        CacheFileData_t* pCacheFileData = (m_cacheFile.begin())->second;
        if (pCacheFileData) {

          uint32_t nBlockCacheSize = (uint32_t)pCacheFileData->size();
          if (nBlockCacheSize > 1 ) { pCacheFileData->ShrinkToSize(nBlockCacheSize - 1); continue; }
        }
      }

      // extern
      if (size > MemoryPoolThreadSafe::DEFAULT_GRANULARITY_SIZE) {
        m_nPoolSize += ((size + MemoryPoolThreadSafe::DEFAULT_GRANULARITY_SIZE - 1) 
          / MemoryPoolThreadSafe::DEFAULT_GRANULARITY_SIZE)
          * MemoryPoolThreadSafe::DEFAULT_GRANULARITY_SIZE;
      }
      else {
        m_nPoolSize += MemoryPoolThreadSafe::DEFAULT_GRANULARITY_SIZE;
      }

      m_poolFileDataMemory.resize(m_nPoolSize);
    }
    */
  }

  return node;
}

//////////////////////////////////////////////////////////////////////////
void CacheFileData::clearCacheData() {

  /*
  CacheFile_t::iterator it_cache, end;
  for (it_cache = m_cacheFile.begin(), end = m_cacheFile.end(); it_cache != end; ++it_cache) {

    CacheFileData_t* pCacheFileData = it_cache->second;
    if (NULL == pCacheFileData) { continue; }

    size_t nBlockCacheSize = pCacheFileData->size();
    if (nBlockCacheSize) { pCacheFileData->ShrinkToSize(nBlockCacheSize - 1); }
  }
  */

  m_cacheFile.Clear();
}

END_NAMESPACE_AGGREGATOR
