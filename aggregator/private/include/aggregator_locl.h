/*

*/

#ifndef AGGREGATOR_LOCL_H_
#define AGGREGATOR_LOCL_H_

#include "aggregator_base.h"

BEGIN_NAMESPACE_AGGREGATOR

interface IFileContainer;
interface INetHandler;

interface IAggregator : public IAggregatorBase {
public:
  virtual IFileContainer* GetFileContainer(const char_t* strType) = 0;

public:
  virtual rc_t Init(IHostContext*) = 0;
  virtual void DeInit() = 0;

  virtual rc_t Start() = 0;
  virtual void Stop() = 0;

public:
  virtual uint32_t GetTick() = 0;
  virtual uint32_t GetLongTick() = 0;

  virtual rc_t TickProc() = 0;  
  virtual rc_t LongTickProc() = 0;
  virtual rc_t NetProc(INetHandler*, const uint8_t* data, uint32_t len) = 0;

public:
  virtual rc_t AddFile(const file_id_t&, const char_t* collector
    , const char_t* strDir, const char_t* strName, uint64_t create_time) = 0;
  virtual rc_t SetData(const file_id_t&, IMemoryNode* memNode, file_size_t pos, file_size_t len) = 0;

  // old file
  virtual rc_t AddFile(const file_id_t& file_id, const char_t* collector, const char_t* strLocalPath
    , const char_t* strDir, const char_t* strName, uint64_t create_time
    , file_size_t size, bool_t bFinish, const char_t* checksum) = 0;

public:
  AGGREGATOR_EXPORT
  static IAggregator* CreateInstance();
}; // IAggregator

interface IFileIterator : public IFileIteratorBase {
public:
  virtual bool_t Wait(uint32_t time) = 0;
  virtual void Signal() = 0;
}; // IFileIterator


//////////////////////////////////////////////////////////////////////////
EXTERN_C AGGREGATOR_EXPORT
IAggregator* Aggregator_CreateInstance();

END_NAMESPACE_AGGREGATOR

#endif // AGGREGATOR_LOCL_H_
