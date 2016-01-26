/*
  net unittest
*/
#define ENABLE_DETECT_MEMORY_LEAK
#include "test.h"

#ifdef DO_UNITTEST

TEST_DEFINE(net, interface_list);

EXTERN_C SYMBOL_EXPORT
TEST_MAIN_MOCK_BEGIN(net, unittest) {

  memory_leak_unittest();

  TEST_EXEC(net, interface_list);

} TEST_MAIN_MOCK_END

#endif // DO_UNITTEST
