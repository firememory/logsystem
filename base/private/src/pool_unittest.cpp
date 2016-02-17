/*

*/

#include "test.h"

#ifdef DO_UNITTEST

#include "pool.h"
USING_NAMESPACE_BASE;

template<typename _P>
void pool_test(_P& IntPool) {

  IntPool.init(6, 11);
  int* arrPoint[1024] = {NULL};

  for (uint32_t idx = 0; idx < 200; ++idx) {

    int* pInt = IntPool.get();
    ASSERT_NE(NULL, pInt);

    if (idx == 32) {
      IntPool.release(pInt);
    }
    else {
      arrPoint[idx] = pInt;
    }
  }

  // OK
  int* pIntEx = NULL;
  {
    pIntEx = IntPool.get();
    ASSERT_NE(NULL, pIntEx);
  }

  // NOT_FOUND
  {
    int* pInt = IntPool.get();
    ASSERT_EQ(NULL, pInt);
  }

  // release
  {
    for (uint32_t idx = 0; idx < 1024; ++idx) {

      if (arrPoint[idx]) { IntPool.release(arrPoint[idx]); }
    }
  }
  {
    IntPool.release(pIntEx);
  }

  // dead
//   {
//     // assert
//     int* pInt = reinterpret_cast<int*>(0x12345678);
//     IntPool.release(pInt);
//   }
}


TEST_BEGIN(base, pool_no_threadsafe) {

  typedef PoolNoThreadSafe<int, int> int_pool_t;
  int_pool_t intPool(200, 10);
  pool_test<int_pool_t>(intPool);

} TEST_END

TEST_BEGIN(base, pool_threadsafe) {

  typedef PoolThreadSafe<int, int> int_poolts_t;
  int_poolts_t intPool(200, 10);
  pool_test<int_poolts_t>(intPool);
  
} TEST_END

TEST_BEGIN(base, pool) {

  TEST_EXEC(base, pool_no_threadsafe);
  TEST_EXEC(base, pool_threadsafe);

} TEST_END


#endif // DO_UNITTEST
