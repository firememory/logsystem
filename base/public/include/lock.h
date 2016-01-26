/*

*/

#ifndef LOCK_H_
#define LOCK_H_

#include "base.h"
#include "base_export.h"

BEGIN_NAMESPACE_BASE

class BASE_EXPORT Lock {
public:
  Lock();
  ~Lock();

protected:
  Lock(bool_t)
    : m_handle(NULL)
  {}

public:
  inline void AssertAcquired() const {}

public:
  void Acquire();
  void Release();
  bool_t Try();

public:
  inline bool_t IsValid() { return NULL == m_handle ? FALSE : TRUE; }

private:
  typedef void*     handle_t;

  handle_t    m_handle;

  DISALLOW_COPY_AND_ASSIGN(Lock);
}; // Lock
/*
class BASE_EXPORT LockMock : public Lock {
public:
  LockMock()
    : Lock(TRUE)
  {}

public:
  inline void AssertAcquired() const {}

public:
  inline void Acquire() {}
  inline void Release() {}
  inline bool_t Try() { return TRUE; }

public:
  inline bool_t IsValid() { return TRUE; }
}; // LockMock
*/
// A helper class that acquires the given Lock while the AutoLock is in scope.
class BASE_EXPORT AutoLock {
public:
  /*explicit */AutoLock(Lock& lock) : lock_(lock) {
    lock_.Acquire();
  }

  ~AutoLock() {
    lock_.AssertAcquired();
    lock_.Release();
  }

private:
  Lock& lock_;
  DISALLOW_COPY_AND_ASSIGN(AutoLock);
}; // AutoLock

// AutoUnlock is a helper that will Release() the |lock| argument in the
// constructor, and re-Acquire() it in the destructor.
class BASE_EXPORT AutoUnlock {
public:
  /*explicit */AutoUnlock(Lock& lock) : lock_(lock) {
    // We require our caller to have the lock.
    lock_.AssertAcquired();
    lock_.Release();
  }

  ~AutoUnlock() {
    lock_.Acquire();
  }

private:
  Lock& lock_;
  DISALLOW_COPY_AND_ASSIGN(AutoUnlock);
}; // AutoUnlock


END_NAMESPACE_BASE

#endif // LOCK_H_
