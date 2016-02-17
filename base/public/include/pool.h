/*

*/

#ifndef POOL_H_
#define POOL_H_

#include "base.h"
#include "base_export.h"

#include "std_list"

#include "lock.h"

BEGIN_NAMESPACE_BASE

template <typename _Ty, typename _P, typename AllocatorType, typename DeletorType>
class Pool {

public:
  Pool(uint32_t N, uint32_t Pre) : _curr_count(0) { setSize(N, Pre); }

  Pool(uint32_t N, uint32_t Pre, const AllocatorType& allocator, const DeletorType& deletor)
    : allocator_(allocator)
    , deletor_(deletor)
  {
    setSize(N, Pre);
  }

  virtual ~Pool() { clear(); }

  // not thread safe
public:
  void setSize(uint32_t N, uint32_t Pre) {
    _N    = N;
    _Pre  = Pre;
  }

#if defined(DEBUG_ON)
  uint32_t total_size() const { return (uint32_t)_pool_container_org.size(); }
#else
  uint32_t total_size() const { return _N; }
#endif
  uint32_t free_size() const { return (uint32_t)_pool_container.size(); }

  // only can do after construct..
  // not thread safe
  void init(_P param, uint32_t count = 0) {

    // save param
    _param = param;
    init_(count);
  }

  void clear() {

#if defined(DEBUG_ON)
    ASSERT(_pool_container_org.size() == _pool_container.size());
    _pool_container_org.clear();
#endif // DEBUG_ON

    PoolContainer::iterator it, end;
    for (it = _pool_container.begin(), end = _pool_container.end(); it != end; ++it) {
      deletor_(*it);
      //delete (*it);
    }
    _pool_container.clear();
  }

public:
  // for check alive
  _Ty* get_no_create() {

    if (_pool_container.empty()) { return NULL; }

    _Ty* pt = _pool_container.front();
    _pool_container.pop_front();
    return pt;
  }

  _Ty* get() {

    if (_pool_container.empty()) {

      if (_N > _curr_count) {
        uint32_t count = _N - _curr_count;
        count = count > _Pre ? _Pre : count;
        init_(count);
      }
    }

    if (!_pool_container.empty()) {

      _Ty* pt = _pool_container.front();
      _pool_container.pop_front();
      return pt;
    }
    return NULL;
  }

  void release(_Ty* t) {

    ASSERT(t);
    // check
#if defined(DEBUG_ON)
    PoolContainer::const_iterator it, end;
    for (it = _pool_container_org.begin(), end = _pool_container_org.end(); it != end; ++it) {

      if ((*it) == t) {
        break;
      }
    }
    ASSERT(it != _pool_container_org.end());
#endif // DEBUG_ON

    _pool_container.push_back(t);
  }

private:

  void init_(uint32_t count) {

    for (; count; --count) {

      //_Ty* t = new _Ty((void*)(this), _param);
      _Ty* t = allocator_(this, _param);
      if (NULL == t) { continue; }

      _pool_container.push_back(t);
      ++_curr_count;
#if defined(DEBUG_ON)
      _pool_container_org.push_back(t);
      ASSERT(_pool_container_org.size() == _curr_count);
#endif // DEBUG_ON
    }
  }

private:
  typedef std::list<_Ty*>      PoolContainer;

  PoolContainer               _pool_container;
  _P                          _param;

  uint32_t                    _N;
  uint32_t                    _Pre;
  uint32_t                    _curr_count;

  AllocatorType               allocator_;
  DeletorType                 deletor_;

#if defined(DEBUG_ON)
  PoolContainer               _pool_container_org;
#endif // DEBUG_ON

  DISALLOW_COPY_AND_ASSIGN(Pool);
}; // Pool

//////////////////////////////////////////////////////////////////////////
//
template<typename _Ty, typename _P>
struct PoolDefaultAllocator {
public:
  _Ty* operator()(void* pool, _P param) { return new _Ty(pool, param); }
};


template<typename _Ty>
struct PoolDefaultDeletor {
public:
  void operator()(_Ty*& t) { delete t; }
};


template<typename _Ty, typename _P>
class PoolNoThreadSafe : public Pool<_Ty, _P, PoolDefaultAllocator<_Ty, _P>, PoolDefaultDeletor<_Ty> > {
public:
  PoolNoThreadSafe(uint32_t N, uint32_t Pre) 
    : Pool(N, Pre)
  {}
}; // PoolNoThreadSafe
//#define PoolNoThreadSafe                Pool

template <typename _Ty, typename _P>
class PoolThreadSafe : public Pool<_Ty, _P, PoolDefaultAllocator<_Ty, _P>, PoolDefaultDeletor<_Ty> > {
public:
  PoolThreadSafe(uint32_t N, uint32_t Pre) 
    : Pool(N, Pre)
  {}

public:
  _Ty* get_no_create() { AutoLock autolock(_lock);  return Pool::get_no_create(); }
  _Ty* get() { AutoLock autolock(_lock);  return Pool::get(); }

  void release(_Ty* t) { AutoLock autolock(_lock); Pool::release(t); }

private:
  Lock                        _lock;
  DISALLOW_COPY_AND_ASSIGN(PoolThreadSafe);
}; // PoolThreadSafe

END_NAMESPACE_BASE

#endif // POOL_H_
