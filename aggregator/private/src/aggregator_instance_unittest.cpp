/*
  aggregator unittest
*/
#include "test.h"

#ifdef DO_UNITTEST

#include "aggregator_locl.h"
#include "host_context_mock.h"

#include "object.h"
USING_NAMESPACE;

#include "thread.h"
#include "memory_pool.h"
#include "file_util.h"
USING_NAMESPACE_BASE;

USING_NAMESPACE_AGGREGATOR;

TEST_BEGIN(aggregator, instance) {
  char_t* strMemoryLeakCheck = new char_t[64];
  SNPRINTF(strMemoryLeakCheck, 64, 64, "%s:%u", __FUNCTION__, __LINE__);

  MemoryPoolBase poolMemBase;

  NetHandlerMock  NetHandler(2013);
  HostContextMock HostContext(&NetHandler);
  IAggregator* pIAggregator = IAggregator::CreateInstance();

  rc_t rc; 
  rc = pIAggregator->Init(&HostContext);

  // 
  rc = pIAggregator->Start();

  rc = pIAggregator->TickProc();
  rc = pIAggregator->NetProc(&NetHandler,(const uint8_t*)("abcd"), 4);

  rc = pIAggregator->LongTickProc();
  rc = pIAggregator->TickProc();

  static const char_t* strTestFile = _STR("z:/test.txt");
  static const char_t* strTestCollector = _STR("aggregator_unittest");
  file_size_t fileSize = 16*1024;

  rc = pIAggregator->AddFile(1, strTestFile, strTestCollector);
  ASSERT_EQ(RC_S_OK, rc);

  rc = pIAggregator->AddFile(1, strTestFile, strTestCollector);
  ASSERT_NE(RC_S_OK, rc);

  typedef MemoryNode<MemoryPoolBase>      memory_node_t;
  AutoRelease<memory_node_t*> autoRelMemNode(memory_node_t::CreateInstance(poolMemBase, (uint32_t)fileSize));
  if (autoRelMemNode) {

    AutoReleaseFile AutoRelFile(OpenFile(strTestFile, strAttrOpenRead));
    ASSERT_NE(NULL, AutoRelFile);

    
    ASSERT_EQ(RC_S_OK, ReadFile(autoRelMemNode->data(), &fileSize, AutoRelFile, 0));
    autoRelMemNode->Set((uint32_t)fileSize);

    rc = pIAggregator->SetData(1, autoRelMemNode, 0);
    rc = pIAggregator->SetData(1, autoRelMemNode, 0);

    rc = pIAggregator->SetData(3, autoRelMemNode, 0);
    ASSERT_NE(RC_S_OK, rc);
  }

  bool_t bControl = TRUE;
  while (TRUE == bControl) {

    Thread::sleep(1);
  }

  pIAggregator->Stop();

  pIAggregator->DeInit();
  pIAggregator->Release(); 
} TEST_END

#endif // DO_UNITTEST
