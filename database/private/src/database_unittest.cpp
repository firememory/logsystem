/*

*/
#define ENABLE_DETECT_MEMORY_LEAK
#include "test.h"

#ifdef DO_UNITTEST

#include "database.h"

USING_NAMESPACE_DATABASE

TEST_BEGIN(database, impl) {


} TEST_END



TEST_DEFINE(database, null);
TEST_DEFINE(database, mysql);

EXTERN_C SYMBOL_EXPORT
TEST_MAIN_MOCK_BEGIN(database, unittest) {

  detect_memory_leaks(true);
  InitConsoleWindow();

  TEST_EXEC(database, null);
  TEST_EXEC(database, mysql);

  TEST_EXEC(database, impl);
} TEST_MAIN_MOCK_END

#endif // DO_UNITTEST