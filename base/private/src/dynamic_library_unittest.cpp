/*

*/

#include "test.h"

#ifdef DO_UNITTEST

#include "dynamic_library.h"
USING_NAMESPACE_BASE;

static bool_t gbCall = FALSE; 

EXTERN_C SYMBOL_EXPORT
TEST_BEGIN(base, dynamic_library_test) {

  uint32_t nValue = 0;
  nValue += 3;

  gbCall = TRUE;

} TEST_END

TEST_BEGIN(base, dynamic_library) {

  DynamicLibrary dync;
  rc_t rc = dync.open(_STR("base_unittest.dll"));
  ASSERT_EQ(RC_S_OK, rc);

  typedef void (*PFN_TEST_CALL)();
  PFN_TEST_CALL pfn_call = dync.LocateSymbol<PFN_TEST_CALL>(_STR("base_dynamic_library_test"));
  ASSERT_NE(NULL, pfn_call);

  (pfn_call)();

  ASSERT_EQ(TRUE, gbCall);

} TEST_END


#endif // DO_UNITTEST
