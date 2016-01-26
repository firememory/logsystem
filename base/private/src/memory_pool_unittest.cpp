/*

*/
#include "test.h"

#ifdef DO_UNITTEST
#include "memory_pool.h"
USING_NAMESPACE_BASE;


rc_t find_memory(const void** check, uint32_t size, const void* mem) {

  for (uint32_t idx = 0; idx < size; ++idx) {
    if (mem == check[idx]) { return RC_S_OK; }
  }
  return RC_S_NOTFOUND;
}

template<typename _P>
void memory_pool_test(_P& memPool) {

  ASSERT_NE(FALSE, memPool.isValid());
  TRACE("Total Size = %u", memPool.total_size());
  TRACE("Free Size = %u", memPool.free_size());

  void* arrMem[2048] = {NULL};
  uint32_t alloc_count = 0;
  {
    void* arrMemCheck[2048] = {NULL};
    for (uint32_t idx = 0; idx < 512; ++idx) {

      void* mem = NULL;
      mem = memPool.get(64);
      if (NULL == mem) {
        TRACE("Total Size = %u", memPool.total_size());
        TRACE("Free Size = %u", memPool.free_size());
        continue;
      }

      arrMem[alloc_count] = mem;
      arrMemCheck[alloc_count] = mem;
      alloc_count++;
    }

    uint32_t reuse_count = 0;
    uint32_t release_count = 0;
    // clear
    for (uint32_t idx = 0; idx < alloc_count; ++idx) {

      void* mem = arrMem[idx];
      ASSERT_NE(NULL, mem);
      memPool.release(mem);
      arrMem[idx] = NULL;

      ++idx;
      ++release_count;
    }
    
    // again
    for (uint32_t idx = 0; idx < 512; ++idx) {

      void* mem = NULL;
      mem = memPool.get(64);
      if (NULL == mem) { continue; }
      if (RC_S_OK == find_memory((const void**)(&arrMemCheck), sizeof(arrMemCheck), mem)) {
        ++reuse_count;
      }

      arrMem[alloc_count] = mem;
      alloc_count++;
    }

    ASSERT_EQ(release_count, reuse_count);
    TRACE("reuse count = %u", reuse_count);
  }

  {
    void* mem = NULL;
    memPool.release(mem);
    mem = NULL;
  }

  {
    void* mem = NULL;
    mem = memPool.get(65*1024);
    //ASSERT_EQ(NULL, mem);
    arrMem[alloc_count] = mem;
    alloc_count++;
  }

  // release all
  {
    for (uint32_t idx = 0; idx < alloc_count; ++idx) {

      void* mem = arrMem[idx];
      if (mem) {
        memPool.release(mem);
      }
    }
  }
}

TEST_BEGIN(base, memory_pool_base) {

  MemoryPool memPool(4*1024*1024);

  void* mem = NULL;
  mem = memPool.get(64);
  if (NULL == mem) { TRACE(_STR("Alloc Failed")); }
  else {
    TRACE("Total Size = %u", memPool.total_size());
    TRACE("Free Size = %u", memPool.free_size());
    memPool.release(mem);
  }
  

} TEST_END

TEST_BEGIN(base, memory_pool_no_threadsafe) {

  MemoryPool poolMem(1024);
  memory_pool_test(poolMem);

} TEST_END

TEST_BEGIN(base, memory_pool_threadsafe) {

  MemoryPoolThreadSafe poolMem(1024);
  memory_pool_test(poolMem);

} TEST_END

TEST_BEGIN(base, memory_pool) {

  TEST_EXEC(base, memory_pool_base);
  TEST_EXEC(base, memory_pool_no_threadsafe);
  TEST_EXEC(base, memory_pool_threadsafe);

} TEST_END

#endif // DO_UNITTEST
