/*

*/

#include "memory_pool.h"

#ifdef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN__UNDEF
#undef WIN32_LEAN_AND_MEAN
#endif

#define MSPACES                         1
#define ONLY_MSPACES                    1
#define USE_LOCKS                       1
#define FOOTERS                         1

#if defined(DEBUG_ON)
# define DEBUG                          1
#else
# define DEBUG                          0
#endif

/* The default of 64Kb means we spend too much time kernel-side */
// #ifndef DEFAULT_GRANULARITY
// #define DEFAULT_GRANULARITY (1*1024*1024)
// #endif
/*#define USE_SPIN_LOCKS 0*/

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4127)
# define interlockedexchange(t,v) _InterlockedExchange((volatile LONG *)(t), (v))
#endif // _MSC_VER

#include "malloc.c"

#ifdef _MSC_VER
# undef interlockedexchange
# pragma warning(default: 4127)
#pragma warning(pop)
#endif // _MSC_VER


#ifdef WIN32_LEAN_AND_MEAN__UNDEF
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN__UNDEF

#undef DEBUG
#undef FOOTERS
#undef USE_LOCKS
#undef ONLY_MSPACES
#undef MSPACES

BEGIN_NAMESPACE_BASE

//////////////////////////////////////////////////////////////////////////
MemoryPool::MemoryPool(size_t size) 
  : m_mspace(create_mspace(DEFAULT_GRANULARITY_SIZE, 1))
#if defined(DEBUG_ON)
  , m_nAllocCount(0)
  , m_nTatolAlloc(0)
#endif // DEBUG_ON
{
  resize(size);
}

MemoryPool::~MemoryPool() {

  if (m_mspace) { 
#if defined(DEBUG_ON)
    struct mallinfo mi = mspace_mallinfo(m_mspace);
    ASSERT(m_uordblks == mi.uordblks);
#endif // DEBUG_ON
    destroy_mspace(m_mspace); 
  }
}

rc_t MemoryPool::resize(size_t size) {

  if (NULL == m_mspace || 0 == size) { return RC_S_NULL_VALUE; }
  mspace_set_footprint_limit(m_mspace, size);

#if defined(DEBUG_ON)
  struct mallinfo mi = mspace_mallinfo(m_mspace);
  m_uordblks = mi.uordblks;
#endif // DEBUG_ON

  return RC_S_OK;
}

void* MemoryPool::get(size_t size) {

  ASSERT(m_mspace);
  /*
  const size_t kMIN_ALLOC_SIZE          = 64 * 1024;
  size_t real_size = ((size + kMIN_ALLOC_SIZE - 1) / kMIN_ALLOC_SIZE) * kMIN_ALLOC_SIZE;
  void* pMem = mspace_malloc(m_mspace, real_size);

  if (pMem) { BZERO(pMem, real_size); }
  return pMem;
  */

  const size_t kMIN_ALLOC_SIZE          = sizeof(void*);
  size_t real_size = (size_t)size + kMIN_ALLOC_SIZE;
  void* pMem = mspace_malloc(m_mspace, real_size);
  if (pMem) {

#if defined(DEBUG_ON)
    ++m_nAllocCount;
    m_nTatolAlloc += real_size;
#endif

    BZERO((uint8_t*)pMem + size, kMIN_ALLOC_SIZE);
  }

  return pMem;
}  

void MemoryPool::release(void* mem) {

  ASSERT(m_mspace);
  mspace_free(m_mspace, mem);
}

//////////////////////////////////////////////////////////////////////////
size_t MemoryPool::total_size() const {
  
  ASSERT(m_mspace);
  struct mallinfo mi = mspace_mallinfo(m_mspace);
  return mi.usmblks;
}

size_t MemoryPool::free_size() const {

  ASSERT(m_mspace);
  struct mallinfo mi = mspace_mallinfo(m_mspace);
  return mi.fordblks;
}

END_NAMESPACE_BASE

