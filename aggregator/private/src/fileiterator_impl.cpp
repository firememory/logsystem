/*

*/

#include "fileiterator_impl.h"

BEGIN_NAMESPACE_AGGREGATOR

void FileIteratorImpl::Release() {

  ASSERT(m_poolFileIterator); 
  releaseFileContainer();
  m_poolFileIterator->release(this);
}

rc_t FileIteratorImpl::file_iterate(IFileIteratorCallBack* pIFileIteratorCallBack) {

  if (NULL == pIFileIteratorCallBack) { return RC_S_NULL_VALUE; }
  if (NULL == m_pIFileSystem || NULL == m_autoRelIFileContainer) { return RC_S_NOTFOUND; }
  if (NULL == m_pFileContainer || 0 == m_nFCSize) { return RC_S_NOTFOUND; }

  rc_t rc = RC_S_OK;
  size_t nFCCount = m_nFCSize;

  pIFileIteratorCallBack->Begin(this);

  if (RC_S_OK == m_autoRelIFileContainer->CopyContainer(m_pFileContainer, &nFCCount)) {

    size_t nLoopCount = min(nFCCount, m_nFCSize);
    for (size_t idx = 0; idx < nLoopCount; ++idx) {

      file_id_t file_id = m_pFileContainer[idx];

      // quick release file node
      {
        AutoRelease<IFileNode*> autoRelFileNode(m_pIFileSystem->GetFileNode(file_id));
        if (NULL == autoRelFileNode) { continue; }

        rc = pIFileIteratorCallBack->FileProc(this, autoRelFileNode);
      }

      if (RC_S_CLOSED == rc) { m_pIFileSystem->FileOver(file_id); }
      else if (RC_S_OK != rc) { break; }
    }

    if (nFCCount > m_nFCSize) {
      free(m_pFileContainer);
      m_pFileContainer = NULL;

      m_pFileContainer = (file_id_t*)malloc(nFCCount * 2);
      if (m_pFileContainer) { m_nFCSize = 2 * nFCCount; }
    }
  }
  /*
  const IFileContainer::file_list_t& lstLogFile = m_autoRelIFileContainer->Container();
  IFileContainer::file_list_t::const_iterator it_list, end;
  for (it_list = lstLogFile.begin(), end = lstLogFile.end(); RC_S_OK == rc && it_list != end; ++it_list) {

    file_id_t file_id = (*it_list);

    // quick release file node
    {
      AutoRelease<IFileNode*> autoRelFileNode(m_pIFileSystem->GetFileNode(file_id));
      if (NULL == autoRelFileNode) { continue; }

      rc = pIFileIteratorCallBack->FileProc(this, autoRelFileNode);
    }

    if (RC_S_CLOSED == rc) { m_pIFileSystem->FileOver(file_id); }
    else if (RC_S_OK != rc) { break; }
  }
  */
  pIFileIteratorCallBack->End();
  m_event.Wait();
  
  return RC_S_OK;
}

IMemoryNode* FileIteratorImpl::GetFileData(file_id_t file_id, file_size_t* pos) {

  if (NULL == m_pIFileSystem || NULL == pos) { return NULL; }

  // get more data
  AutoRelease<IFileNode*> autoRelFileNode(m_pIFileSystem->GetFileNode(file_id));
  if (NULL == autoRelFileNode) { return NULL; }

  return m_pIFileSystem->GetData(file_id, pos, autoRelFileNode->Size() - (*pos));
}

//////////////////////////////////////////////////////////////////////////
static const size_t kDefaultFCSize = 64 * 1024;
//////////////////////////////////////////////////////////////////////////
FileIteratorImpl::~FileIteratorImpl() {

  if (m_pFileContainer && m_nFCSize) { 
    free(m_pFileContainer);
    m_pFileContainer = NULL;
    m_nFCSize = 0;
  }
}

FileIteratorImpl::FileIteratorImpl(void* pool, const FileIteratorParam_t& fileIteratorParam)
  : m_pIAggregator(fileIteratorParam.pIAggregator)
  , m_pIFileSystem(fileIteratorParam.pIFileSystem)
  , m_poolFileIterator(static_cast<FileIterator_pool_t*>(pool))
  , m_event(FALSE, FALSE)
  , m_autoRelIFileContainer(NULL)

  , m_pFileContainer(NULL)
  , m_nFCSize(0)
{
  m_pFileContainer = (file_id_t*)malloc(kDefaultFCSize * sizeof(file_id_t));
  if (m_pFileContainer) { m_nFCSize = kDefaultFCSize; }
}

void FileIteratorImpl::SetType(const char_t* strType) {

  if (NULL == strType) { return; }
  STRCPY(m_strType, kMAX_LOGTYPE_LEN, strType);

  updateFileContainer();
}

void FileIteratorImpl::releaseFileContainer() {

  if (m_autoRelIFileContainer) {
    m_autoRelIFileContainer->DetachFileIterator(this);
    m_autoRelIFileContainer.Release();
  }
}

void FileIteratorImpl::updateFileContainer() {

  //releaseFileContainer();

  if (NULL == m_pIAggregator) { return; }

  if (m_autoRelIFileContainer) { return; }

  m_autoRelIFileContainer.Set(m_pIAggregator->GetFileContainer(m_strType));
  if (NULL == m_autoRelIFileContainer) { return; }
  m_autoRelIFileContainer->AttachFileIterator(this);
}

bool_t FileIteratorImpl::Wait(uint32_t time) {

  bool_t rc = TRUE;

  if (0 == time) { m_event.Wait(); }
  else {
    rc = m_event.TimedWait(time);
  }

  updateFileContainer();
  return rc;
}

void FileIteratorImpl::Signal() {

  m_event.Signal();
}

END_NAMESPACE_AGGREGATOR
