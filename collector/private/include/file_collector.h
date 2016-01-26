/*

*/

#ifndef FILE_COLLECTOR_H_
#define FILE_COLLECTOR_H_

#include "collector.h"
#include "collector_export.h"

#include "aggregator_base.h"

BEGIN_NAMESPACE_COLLECTOR

static const char_t* strMemMiniLog             = _STR("MemMiniLog");
static const uint32_t kstrMemMiniLogLen = 10;

typedef enum collector_log_type_e_ {
  FULL_LOG
  , MEM_NINI_LOG
} collector_log_type_e;


USING_AGGREGATOR(IFileNode);
USING_AGGREGATOR(file_id_t);

interface IFileCollector {
public:
  enum { kMAX_READ_SIZE = 256 * 1024, kMAX_DEFALT_BUF_SIZE = 1024 * 1024 };

public:
  virtual void Release() = 0;

public:
  virtual rc_t AddCollectRule(const char_t* strDir, const char_t* strExclude, const char_t* strInclude) = 0;
  virtual void ClearCollectRule() = 0;

  virtual IFileNode* AddFileNode(const file_id_t&, file_size_t pos, bool_t bFinish, const char_t* strDir, const char_t* strName) = 0;

  virtual IFileNode* FindFileNode(const char_t* strDir, const char_t* strName) = 0;
  virtual IFileNode* FindFileNode(const file_id_t&) = 0;
  virtual void ClearFileNode() = 0;

  //virtual void ConfirmSend() = 0;
  virtual void SetTimeControl(bool_t bAfterToDay) = 0;
public:
  /*
    RC_S_OK   ==> continue
  */
  typedef rc_t (*file_collect_callback_t)(const IFileNode*, void* context);
  virtual rc_t Collector(collector_log_type_e, file_collect_callback_t, void* context) = 0;

public:
  virtual void TickProc() = 0;
}; // IFileCollector

END_NAMESPACE_COLLECTOR
#endif // FILE_COLLECTOR_H_
