/*
  base unittest
*/
#define ENABLE_DETECT_MEMORY_LEAK
#include "test.h"

#ifdef DO_UNITTEST

TEST_DEFINE(base, atomic_count);
TEST_DEFINE(base, dynamic_library);
TEST_DEFINE(base, time);

TEST_DEFINE(base, thread);
TEST_DEFINE(base, lock);
TEST_DEFINE(base, waitable_event);

TEST_DEFINE(base, pool);
TEST_DEFINE(base, memory_pool);

TEST_DEFINE(base, stl);

EXTERN_C SYMBOL_EXPORT
TEST_MAIN_MOCK_BEGIN(base, unittest) {

  memory_leak_unittest();

  TEST_EXEC(base, stl);

  TEST_EXEC(base, atomic_count);
  TEST_EXEC(base, dynamic_library);
  TEST_EXEC(base, time);

  TEST_EXEC(base, thread);
  TEST_EXEC(base, lock);
  TEST_EXEC(base, waitable_event);

  TEST_EXEC(base, pool);
  TEST_EXEC(base, memory_pool);
} TEST_MAIN_MOCK_END

#endif // DO_UNITTEST
