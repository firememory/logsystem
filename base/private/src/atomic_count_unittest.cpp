/*

*/

#include "test.h"

#ifdef DO_UNITTEST

#include "atomic_count.h"
USING_NAMESPACE_BASE;

TEST_BEGIN(base, atomic_count) {

  atomic_count acOne(1);
  uint32_t nValue;
  nValue = ++acOne;
  nValue = --acOne;

} TEST_END

#endif // DO_UNITTEST
