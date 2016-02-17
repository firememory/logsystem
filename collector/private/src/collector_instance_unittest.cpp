/*
  collector unittest
*/
#include "test.h"

#ifdef DO_UNITTEST

#include "log_collector_impl.h"
USING_NAMESPACE_COLLECTOR

#include "host_context_mock.h"
USING_NAMESPACE_AGGREGATOR

#include "object.h"
USING_NAMESPACE

TEST_BEGIN(collector, instance) {

  NetHandlerMock  NetHandler(2013);
  HostContextMock HostContext(&NetHandler);
  AutoRelease<LogCollectorImpl*> autoRelCollector(LogCollectorImpl::CreateInstance(&HostContext, &NetHandler));
  ASSERT_NE(NULL, autoRelCollector);

  ASSERT_EQ(RC_S_OK, autoRelCollector->Start());


  // file
  autoRelCollector->addCollectRule("z:\\", NULL_STR, "*\\.txt");
  autoRelCollector->addCollectRule("z:", NULL_STR, "[\\u4e00-\\u9fa5]+");
  autoRelCollector->addCollectRule("z://", NULL_STR, "abcd\\.dat");
  autoRelCollector->addCollectRule("z:/", NULL_STR, "[0-9]{4}\\.log");
  autoRelCollector->addCollectRule("z:", NULL_STR, "[a-z]*");  


  autoRelCollector->fileCollect();

  autoRelCollector->PacketTest();

  bool_t bControl = TRUE;
  while (TRUE == bControl) {

    if (RC_S_OK != autoRelCollector->Run()) {

      TRACE("collector run failed");
      break;
    }
  }

} TEST_END

#endif // DO_UNITTEST
