/*

*/

#include "filesystem_impl.h"

BEGIN_NAMESPACE_AGGREGATOR
//////////////////////////////////////////////////////////////////////////

const uint32_t kMAX_READ_SIZE           = 1 * 1024 * 1024;

//////////////////////////////////////////////////////////////////////////
const char_t* FileSystemImpl::m_strName = _STR("FileSystem");
//////////////////////////////////////////////////////////////////////////
// resource
const rc_t FileSystemImpl::GetName(char_t* strName, uint32_t* size) const {
  if (NULL == strName || NULL == size || 0 == (*size)) { return RC_S_NULL_VALUE; }
  STRCPY(strName, (*size), m_strName);
  return RC_S_OK;
}

const rc_t FileSystemImpl::GetStatus() const { return RC_S_OK; }

const rc_t FileSystemImpl::GetResourceSize(uint64_t* curr_size, uint64_t* free_size) const {

  if (NULL == curr_size || 0 == free_size) { return RC_S_NULL_VALUE; }

  (*curr_size) = 0x00;
  (*free_size) = 0x00;
  return RC_S_OK;
}

const rc_t FileSystemImpl::GetLastErr(uint32_t* err_no, char_t* err_msg, uint32_t* size) const {

  if (NULL == err_no || NULL == err_msg || NULL == size || 0 == (*size)) { return RC_S_NULL_VALUE; }

  (*err_no) = 0;
  STRCPY(err_msg, (*size), NULL_STR);
  return RC_S_OK;
}

const rc_t FileSystemImpl::GetInfo(char_t* info, uint32_t* size) const {

  if (NULL == info || NULL == size || 0 == (*size)) { return RC_S_NULL_VALUE; }

  uint32_t nFileCount = 0;
  uint32_t nFileBlockCount = 0;

  m_cacheFileBuffer.GetCacheCount(&nFileCount, &nFileBlockCount);

  size_t nTotalSize = 0;
  size_t nFreeSize = 0;
  m_cacheFileBuffer.GetMemorySize(&nTotalSize, &nFreeSize);

  (*size) = SNPRINTF(info, (*size), (*size), _STR("%s: \n"
    "File Count=%u\n"
    "Cache FileCount=%u, FileBlockCount=%u\n"
    "Cache Memory TotalSize=%u, FreeSize=%u\n"
    "Cache Get Memory Failed Count=%u\n"
    )

    , STR_VERSION
    , m_mapFileNode.size()
    , nFileCount, nFileBlockCount
    , nTotalSize, nFreeSize
    , m_cacheFileBuffer.GetCountGetMemFailed()
    );
  return RC_S_OK;
}

const rc_t FileSystemImpl::GetLastAliveTime(uint32_t* alive) const {

  if (NULL == alive) { return RC_S_NULL_VALUE; }

  (*alive) = m_timeAlive;
  return RC_S_OK;
}

//////////////////////////////////////////////////////////////////////////
IMemoryNode* FileSystemImpl::GetData(file_id_t file_id, file_size_t* pos, file_size_t size) {

  ASSERT(size < kuint32max);
  ASSERT(pos);
  //if (NULL == memNode) { return RC_S_NULL_VALUE; }

  m_timeAlive = DayTime::now();

  IMemoryNode* memNode = NULL;
  file_size_t real_pos = (*pos);
  if (RC_S_OK == m_cacheFileBuffer.Get(&memNode, file_id, &real_pos)) { 
    (*pos) = real_pos;
    return memNode;
  }

  // next pos == real_pos

  // find
  AutoRelease<FileNodeImpl*> autoRelFileNode(findFileNode(file_id));
  if (NULL == autoRelFileNode) { return NULL/*RC_S_NOTFOUND*/; }
  if ((*pos) >= autoRelFileNode->Size()) { return NULL/*RC_E_INDEX*/; }

  // read file
  // memory
  size = size - (real_pos - (*pos));
  size = size > kMAX_READ_SIZE ? kMAX_READ_SIZE : size;

  AutoRelease<IMemoryNode*> autoMemNode(m_cacheFileBuffer.GetMemory((uint32_t)size));
  if (NULL == (IMemoryNode*)autoMemNode) { return NULL/*RC_E_NOMEM*/; }

  ASSERT(size == autoMemNode->len());

  ASSERT(real_pos >= (*pos));
  //size = size - (real_pos - (*pos));
  //file_size_t read_size = size > kMAX_READ_SIZE ? kMAX_READ_SIZE : size;
  file_size_t read_size = size;
  rc_t rc = autoRelFileNode->GetData(autoMemNode->data(), &read_size, (*pos));
  if (RC_S_OK != rc) { return NULL/*rc*/; }

  // add to cache
  (void) m_cacheFileBuffer.Put(file_id, (*pos), autoMemNode);

  // return
  autoMemNode->AddRef();
  //*memNode = autoMemNode;
  return autoMemNode/*RC_S_OK*/;
}
/*
IFileContainer* FileSystemImpl::GetFileContainer(const char_t* strType) {

  logtype_map_t::iterator it_map = m_mapLogType.find(strType);
  if (m_mapLogType.end() == it_map) {
    return NULL;
  }

  it_map->second->AddRef();
  return it_map->second;
}
*/
IFileNode* FileSystemImpl::GetFileNode(file_id_t file_id) {

  filenode_map_t::iterator it_map = m_mapFileNode.find(file_id);
  if (m_mapFileNode.end() == it_map) {
    return NULL;
  }

  m_timeAlive = DayTime::now();

  it_map->second->AddRef();
  return it_map->second;
}

void FileSystemImpl::CloseAllFile() {

  filenode_map_t::iterator it_map, end;
  for (it_map = m_mapFileNode.begin(), end = m_mapFileNode.end(); it_map != end; ++it_map) {

    FileNodeImpl* pFileNodeImpl = it_map->second;
    if (NULL == pFileNodeImpl) { continue; }

    pFileNodeImpl->TickProc();
  }
}


rc_t FileSystemImpl::FileOver(file_id_t file_id) {

  AutoRelease<FileNodeImpl*> autoRelFileNode(findFileNode(file_id));
  if (NULL == autoRelFileNode) { return RC_S_NOTFOUND; }

  return autoRelFileNode->OverFile();
}

const IFileNode* FileSystemImpl::AddFile(file_id_t file_id, const char_t* collector, 
                                         const char_t*strDir, const char_t* strName, uint64_t create_time) {

  if (NULL == strDir || NULL == strName || NULL == collector) { return NULL/*RC_S_NULL_VALUE*/; }

  // find
  AutoRelease<FileNodeImpl*> autoRelFileNode(findFileNode(file_id));
  if (autoRelFileNode) { return NULL/*RC_S_DUPLICATE*/; }

  // add to MAP
  FileNodeImpl* pFileNodeImpl = new FileNodeImpl(m_strRootPath, file_id, collector, strDir, strName, create_time);
  if (NULL == pFileNodeImpl) { return NULL/*RC_E_NOMEM*/; }
  m_mapFileNode[file_id] = pFileNodeImpl;

  m_timeAlive = DayTime::now();
  return pFileNodeImpl/*RC_S_OK*/;
}

rc_t FileSystemImpl::SetData(file_size_t* pFileSize, file_id_t file_id, IMemoryNode* memNode, file_size_t pos) {

  ASSERT(pFileSize);
  if (NULL == memNode) { return RC_S_NULL_VALUE; }

  AutoRelease<FileNodeImpl*> autoRelFileNode(findFileNode(file_id));
  if (NULL == autoRelFileNode) { return RC_S_NOTFOUND; }

  ASSERT(autoRelFileNode->ID() == file_id);
  //
  rc_t rc = updateFileNode(autoRelFileNode, memNode, pos);
  if (RC_S_OK != rc) { return rc; }

  (*pFileSize) = autoRelFileNode->Size();

  m_timeAlive = DayTime::now();
  return RC_S_OK;
}

const IFileNode* FileSystemImpl::AddFile(const file_id_t& file_id, const char_t* collector, const char_t* strLocalPath
                                        , const char_t* strDir, const char_t* strName, uint64_t create_time, file_size_t size
                                        , bool_t bFinish, const char_t* checksum) {

  if (NULL == strDir || NULL == strName || NULL == collector || NULL == strLocalPath) { return NULL/*RC_S_NULL_VALUE*/; }

  // find
  AutoRelease<FileNodeImpl*> autoRelFileNode(findFileNode(file_id));
  if (autoRelFileNode) { return NULL/*RC_S_DUPLICATE*/; }

  // add to MAP
  FileNodeImpl* pFileNodeImpl = new FileNodeImpl(file_id, collector, strLocalPath
    , /*strDir, strName, */create_time, size, bFinish, checksum);
  if (NULL == pFileNodeImpl) { return NULL/*RC_E_NOMEM*/; }

  m_mapFileNode[file_id] = pFileNodeImpl;
  return pFileNodeImpl/*RC_S_OK*/;
}

void FileSystemImpl::SetRootDir(const char_t* strRootPath) {
  STRCPY(m_strRootPath, kMAX_PATH, strRootPath);
}

void FileSystemImpl::TickProc() {

  filenode_map_t::iterator it_map, end;
  for (it_map = m_mapFileNode.begin(), end = m_mapFileNode.end(); it_map != end; ++it_map) {

    FileNodeImpl* pFileNodeImpl = it_map->second;
    if (NULL == pFileNodeImpl) { continue; }

    pFileNodeImpl->TickProc();
  }

  m_timeAlive = DayTime::now();
}

//////////////////////////////////////////////////////////////////////////
rc_t FileSystemImpl::updateFileNode(FileSystemImpl::FileNodeImpl* pFileNodeImpl, IMemoryNode* memNode, file_size_t pos) {

  ASSERT(pFileNodeImpl);
  ASSERT(memNode);

  pFileNodeImpl->SetData(memNode, pos);

  // add to file buff
  return m_cacheFileBuffer.Put(pFileNodeImpl->ID(), pos, memNode);
}


FileSystemImpl::FileNodeImpl* FileSystemImpl::findFileNode(file_id_t file_id) {

  filenode_map_t::iterator it_map = m_mapFileNode.find(file_id);
  if (it_map == m_mapFileNode.end()) { return NULL; }

  FileNodeImpl* pFileNode = it_map->second;
  if (NULL == pFileNode 
    || NULL == pFileNode->LocalPath() 
    || NULL == pFileNode->RemotePath() 
    || NULL == pFileNode->Collector()
    //|| pos > pFileNode->Size()
    ) 
  { return NULL; }

  pFileNode->AddRef();
  return pFileNode;
}

FileSystemImpl::FileSystemImpl(size_t max_mem, uint32_t cache_count, uint32_t cache_count_file_block)
  : REF_COUNT_CONSTRUCT
  , m_mapFileNode()
  , m_cacheFileBuffer(max_mem, cache_count, cache_count_file_block)
  , m_timeAlive(0)
{
  BZERO_ARR(m_strRootPath);
}


FileSystemImpl::~FileSystemImpl() {
 
  {
    filenode_map_t::iterator it_map, end;
    for (it_map = m_mapFileNode.begin(), end = m_mapFileNode.end(); it_map != end; ++it_map) {

      FileNodeImpl* pFileNodeImpl = it_map->second;
      delete pFileNodeImpl;
    }
    m_mapFileNode.clear();
  }
}

//////////////////////////////////////////////////////////////////////////
FileSystemImpl* FileSystemImpl::CreateInstanceEx(size_t max_mem, uint32_t cache_count, uint32_t cache_count_file_block) {

  return new FileSystemImpl(max_mem, cache_count, cache_count_file_block);
}

IFileSystem* IFileSystem::CreateInstance(size_t max_mem, uint32_t cache_count, uint32_t cache_count_file_block) {

  return FileSystemImpl::CreateInstanceEx(max_mem, cache_count, cache_count_file_block);
}

END_NAMESPACE_AGGREGATOR
