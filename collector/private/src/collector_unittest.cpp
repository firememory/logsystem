/*
  collector unittest
*/
#define ENABLE_DETECT_MEMORY_LEAK
#include "test.h"

#ifdef DO_UNITTEST


TEST_DEFINE(collector, instance);

EXTERN_C SYMBOL_EXPORT
TEST_MAIN_MOCK_BEGIN(collector, unittest) {

  TEST_EXEC(collector, instance);

} TEST_MAIN_MOCK_END

#endif // DO_UNITTEST
