/*

*/

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "aggregator_base.h"

#include "std_list"

BEGIN_NAMESPACE_AGGREGATOR

interface IFileIterator;
//////////////////////////////////////////////////////////////////////////
interface IFileContainer {
public:
  virtual void Release() = 0;

public:
  virtual rc_t AttachFileIterator(IFileIterator*) = 0;
  virtual rc_t DetachFileIterator(IFileIterator*) = 0;

public:
  typedef std::list<file_id_t>  file_list_t;
  //virtual const file_list_t& Container() = 0;
  virtual rc_t CopyContainer(file_id_t*, size_t*) = 0;
}; // IFileContainer

interface IFileSystem : public IResource {
public:
  virtual void Release() = 0;

public:
  virtual IMemoryNode* GetData(file_id_t file_id, file_size_t* pos, file_size_t size) = 0;

  //virtual IFileContainer* GetFileContainer(const char_t* strType) = 0;
  virtual IFileNode* GetFileNode(file_id_t) = 0;

public:
  virtual const IFileNode* AddFile(file_id_t file_id, const char_t* collector,
    const char_t* strDir, const char_t* strName, uint64_t create_time) = 0;
  virtual rc_t SetData(file_size_t*, file_id_t file_id, IMemoryNode* memNode, file_size_t pos, file_size_t len) = 0;

  virtual const IFileNode* AddFile(const file_id_t& file_id, const char_t* collector, const char_t* strLocalPath
    , const char_t* strDir, const char_t* strName, uint64_t create_time
    , file_size_t size, bool_t bFinish, const char_t* checksum) = 0;
  //virtual rc_t AddLogType(const char_t*) = 0;

  virtual rc_t FileOver(file_id_t file_id) = 0;
  virtual void CloseAllFile() = 0;

  virtual void SetRootDir(const char_t* strRootPath) = 0;

public:
  virtual IMemoryNode* GetMemory(uint32_t size) = 0;

public:
  virtual void TickProc() = 0;

public:
  static IFileSystem* CreateInstance(size_t max_mem);
}; // IFileSystem


END_NAMESPACE_AGGREGATOR

#endif // FILESYSTEM_H_
