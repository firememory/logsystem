/*

*/

#ifndef NOTIFY_QUEUE_H_
#define NOTIFY_QUEUE_H_

#include "base.h"
#include "base_export.h"

#include "object.h"
#include "atomic_count.h"
#include "memory_pool.h"
#include "time_util.h"

#include "std_list"

BEGIN_NAMESPACE_BASE

template<typename _Ty, typename _Ta, typename _Tl>
class NotifyQueue {

  // log
public:
  NotifyQueue() : m_queue(), m_autoRelIListener(NULL) {}
  virtual ~NotifyQueue() {

    // make sure the queue is empty();
    ASSERT(m_queue.empty());
  }

  typedef std::queue<_Ty>            queue_t;

public:
  bool empty() const { _Ta a(m_l); ignore_result(a); return m_queue.empty(); }
  typename queue_t::size_type size() const { return m_queue.size(); }

  typename queue_t::reference front() { _Ta a(m_l); ignore_result(a); return m_queue.front(); }
  typename queue_t::const_reference front() const { _Ta a(m_l); ignore_result(a); return m_queue.front(); }
  typename queue_t::reference back() { _Ta a(m_l); ignore_result(a); return m_queue.back(); }
  typename queue_t::const_reference back() const { _Ta a(m_l); ignore_result(a); return m_queue.back(); }
  void push(const typename queue_t::value_type& _Val) { _Ta a(m_l); ignore_result(a); m_queue.push(_Val); notifyListener(); }
  void pop() { _Ta a(m_l); ignore_result(a); m_queue.pop(); }

public:
  inline rc_t SetListener(IListener* pIListener) {

    _Ta a(m_l); ignore_result(a);
    m_autoRelIListener.Set(pIListener);
    return RC_S_OK;
  }

protected:
  inline void notifyListener() {

    if (NULL == m_autoRelIListener) { return; }
    m_autoRelIListener->Notify();
  }

  // Listener
private:
  AutoRelease<IListener*>     m_autoRelIListener;

  queue_t           m_queue;

  _Tl               m_l;

  DISALLOW_COPY_AND_ASSIGN(NotifyQueue);
}; // NotifyQueue

template<typename _Ty>
class NotifyQueueNoThreadSafe : public NotifyQueue<_Ty, NullClass, NullClass> {
public:
  NotifyQueueNoThreadSafe()
    : NotifyQueue()
  {}
}; // NotifyQueueNoThreadSafe

#include "lock.h"

template<typename _Ty>
class NotifyQueueThreadSafe : public NotifyQueue<_Ty, AutoLock, Lock> {
public:
  NotifyQueueThreadSafe()
    : NotifyQueue()
  {}
}; // NotifyQueueNoThreadSafe

//////////////////////////////////////////////////////////////////////////
END_NAMESPACE_BASE

#endif // NOTIFY_QUEUE_H_
