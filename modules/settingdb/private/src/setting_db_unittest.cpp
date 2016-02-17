/*

*/
#define ENABLE_DETECT_MEMORY_LEAK
#include "test.h"

#ifdef DO_UNITTEST

TEST_BEGIN(module, logtype_db) {

} TEST_END

EXTERN_C SYMBOL_EXPORT
TEST_MAIN_MOCK_BEGIN(module, unittest) {

  TEST_EXEC(module, logtype_db);

} TEST_MAIN_MOCK_END

#endif // DO_UNITTEST
