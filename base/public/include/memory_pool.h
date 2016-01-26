/*

*/

#ifndef MEMORY_POOL_H_
#define MEMORY_POOL_H_

#include "base.h"
#include "base_export.h"

//////////////////////////////////////////////////////////////////////////
BEGIN_NAMESPACE_BASE

//////////////////////////////////////////////////////////////////////////
// MemoryPool STDC
class BASE_EXPORT MemoryPoolBase {
public:
  MemoryPoolBase() {}
  ~MemoryPoolBase() {}

public:
  void* get(size_t size) { return malloc(size); }
  void release(void* mem) { free(mem); }
}; // MemoryPoolBase


class BASE_EXPORT MemoryPool {
public:
  enum { DEFAULT_GRANULARITY_SIZE = 1 * 1024 * 1024 };
  MemoryPool(size_t);
  virtual ~MemoryPool();

public:
  size_t total_size() const;
  size_t free_size() const;
  bool_t isValid() const { return NULL == m_mspace ? FALSE : TRUE; }

public:
  rc_t resize(size_t);
  void* get(size_t);
  void release(void*);

private:
  typedef void*       mspace_t;
  mspace_t    m_mspace;

#if defined(DEBUG_ON)
  size_t              m_nAllocCount;
  size_t              m_nTatolAlloc;
  size_t              m_uordblks;
#endif // DEBUG_ON
  
  DISALLOW_COPY_AND_ASSIGN(MemoryPool);
}; // MemoryPool

typedef MemoryPool                      MemoryPoolNoThreadSafe;

END_NAMESPACE_BASE

//////////////////////////////////////////////////////////////////////////
#include "lock.h"

BEGIN_NAMESPACE_BASE
class BASE_EXPORT MemoryPoolThreadSafe : public MemoryPool {
public:
  MemoryPoolThreadSafe(size_t size)
    : MemoryPool(size)
  {}

public:
  size_t total_size() { AutoLock autoLock(m_lock); return MemoryPool::total_size(); }
  size_t free_size() { AutoLock autoLock(m_lock); return MemoryPool::free_size(); }
  bool_t isValid() { AutoLock autoLock(m_lock); return MemoryPool::isValid(); }

public:
  rc_t resize(size_t size) { AutoLock autoLock(m_lock); return MemoryPool::resize(size); }
  void* get(size_t size) { AutoLock autoLock(m_lock); return MemoryPool::get(size); }
  void release(void* mem) { AutoLock autoLock(m_lock); MemoryPool::release(mem); }

private:
  Lock        m_lock;

  DISALLOW_COPY_AND_ASSIGN(MemoryPoolThreadSafe);
}; // MemoryPoolThreadSafe
END_NAMESPACE_BASE

//////////////////////////////////////////////////////////////////////////
#include "object.h"

#include "atomic_count.h"
BEGIN_NAMESPACE_BASE
// Include Self
template<typename _Ty>
class MemoryNode : public IMemoryNode {
  // IMemoryNode
public:
  void Release() {
    ASSERT(m_acRef);
    if (0 == --m_acRef) { m_poolMem.release(m_data); }
  }

  void AddRef() { ++m_acRef; }

public:
  uint8_t* data() const { return m_data; }
  uint32_t len() const { return (uint32_t)(m_size); }

public:

  static MemoryNode* CreateInstance(_Ty& poolMem, size_t size) {
    uint32_t nodeSize = sizeof(MemoryNode);
    void* mem = poolMem.get(size + nodeSize);
    if (NULL == mem) { return NULL; }
    return new((uint8_t*)(mem) + size) MemoryNode(poolMem, (uint8_t*)mem, size);
  }

  void Set(uint32_t size) { m_size = size; }

private:
  MemoryNode(_Ty& poolMem, uint8_t* data, size_t size)
    : m_poolMem(poolMem)
    , m_size(size)
    , m_data(data)
    , m_acRef(1)
  {}

  ~MemoryNode() { ASSERT(0); }

private:
  _Ty&          m_poolMem;
  size_t        m_size;
  uint8_t*      m_data;

  atomic_count  m_acRef;

  DISALLOW_COPY_AND_ASSIGN(MemoryNode);
}; // MemoryNode

class MemoryNodeStd : public IMemoryNode {
  // IMemoryNode
public:
  void Release() {
    ASSERT(m_acRef);
    if (0 == --m_acRef) { 
      if (m_data) { free(m_data); }
      delete this;
    }
  }

  void AddRef() { ++m_acRef; }

public:
  uint8_t* data() const { return m_data; }
  uint32_t len() const { return m_size; }

public:
  static MemoryNodeStd* CreateInstance(uint32_t size) {
    return new MemoryNodeStd(size);
  }
  void SetSize(uint32_t size) { m_size = size; }

private:
  MemoryNodeStd(uint32_t size)
    : m_size(size)
    , m_data((uint8_t*)malloc(size))
    , m_acRef(1)
  {}

  ~MemoryNodeStd() {}

private:
  uint32_t      m_size;
  uint8_t*      m_data;

  atomic_count  m_acRef;

  DISALLOW_COPY_AND_ASSIGN(MemoryNodeStd);
}; // MemoryNode

END_NAMESPACE_BASE

//////////////////////////////////////////////////////////////////////////
BEGIN_NAMESPACE

typedef AutoRelease<uint8_t*>           AutoReleaseMemoryBase;

template<>
void AutoReleaseMemoryBase::Release() { if (m_handle) { free(m_handle); } }

END_NAMESPACE

#endif // MEMORY_POOL_H_
