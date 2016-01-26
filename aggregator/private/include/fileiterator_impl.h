/*

*/

#ifndef FILEITERATOR_IMPL_H_
#define FILEITERATOR_IMPL_H_

#include "aggregator_locl.h"

#include "waitable_event.h"
#include "pool.h"

#include "filesystem.h"

BEGIN_NAMESPACE_AGGREGATOR
USING_BASE(WaitableEvent);
USING_BASE(PoolThreadSafe);

//////////////////////////////////////////////////////////////////////////
// fileiterator pool
class FileIteratorImpl;

struct FileIteratorParam_t {
  IAggregator* pIAggregator;
  IFileSystem* pIFileSystem;
}; // FileIteratorParam_t
// 
typedef PoolThreadSafe<FileIteratorImpl, FileIteratorParam_t> FileIterator_pool_t;

//////////////////////////////////////////////////////////////////////////
class AGGREGATOR_EXPORT FileIteratorImpl : public IFileIterator {
  // IFileIterator
public:
  void Release();

public:
  /*
  synchronous method 
  */
  rc_t file_iterate(IFileIteratorCallBack*);

public:
  IMemoryNode* GetFileData(file_id_t file_id, file_size_t* pos);

  ///const IFileNode* CurrFileNode() const;

public:
  FileIteratorImpl(void*, const FileIteratorParam_t&);
  ~FileIteratorImpl();

  IFileIterator* ToIFileIterator() { return this; }
  void SetType(const char_t*);

  bool_t Wait(uint32_t time);
  void Signal();

private:
  void updateFileContainer();
  void releaseFileContainer();

private:
  IAggregator*            m_pIAggregator;
  IFileSystem*            m_pIFileSystem;
  FileIterator_pool_t*    m_poolFileIterator;

  char_t                  m_strType[kMAX_LOGTYPE_LEN + 1];

  WaitableEvent           m_event;

  AutoRelease<IFileContainer*>          m_autoRelIFileContainer;

  file_id_t*              m_pFileContainer;
  size_t                  m_nFCSize;

  DISALLOW_COPY_AND_ASSIGN(FileIteratorImpl);
}; // FileIteratorImpl


END_NAMESPACE_AGGREGATOR

#endif // FILEITERATOR_IMPL_H_
