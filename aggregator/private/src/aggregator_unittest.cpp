/*
  aggregator unittest
*/
#define ENABLE_DETECT_MEMORY_LEAK
#include "test.h"

#ifdef DO_UNITTEST

TEST_DEFINE(aggregator, instance);
TEST_DEFINE(aggregator, module);

EXTERN_C SYMBOL_EXPORT
TEST_MAIN_MOCK_BEGIN(aggregator, unittest) {

  TEST_EXEC(aggregator, instance);
  TEST_EXEC(aggregator, module);

} TEST_MAIN_MOCK_END

#endif // DO_UNITTEST
