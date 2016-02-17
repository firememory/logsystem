/*

*/
#define ENABLE_DETECT_MEMORY_LEAK
#include "test.h"

#ifdef DO_UNITTEST

#include "coder.h"
USING_NAMESPACE_CODER;

TEST_BEGIN(coder, impl) {


} TEST_END


TEST_DEFINE(coder, null);
TEST_DEFINE(coder, deflate);

TEST_DEFINE(coder, impl);


EXTERN_C SYMBOL_EXPORT
TEST_MAIN_MOCK_BEGIN(coder, unittest) {

  TEST_EXEC(coder, null);
  TEST_EXEC(coder, deflate);

  TEST_EXEC(coder, impl);
} TEST_MAIN_MOCK_END

#endif // DO_UNITTEST
