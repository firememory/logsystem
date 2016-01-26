/*

*/

#ifndef THRED_H_
#define THRED_H_

#include "base.h"
#include "base_export.h"

BEGIN_NAMESPACE_BASE;

class BASE_EXPORT Thread {
public:
  static void sleep(uint32_t millisecond);

public:
  bool_t Start(void* context);
  bool_t Start(Thread*, void* context);
  void Stop();
  bool_t isRunning() const { return m_bRunning; }
  const char_t* Name() const { return name_; }

#define join          Stop
#define joinable      isRunning

public:
  virtual void ThreadMain(void*) { }

public:
  Thread(const char_t* name);
  ~Thread();

public:
  bool_t Init();
  void DeInit();

private:
  bool_t      m_bStarted;
  bool_t      m_bRunning;

  typedef void*     handle_t;
  handle_t    m_handle;

#define  MAX_NAME_LEN                   64
  char_t name_[MAX_NAME_LEN + 1];

  DISALLOW_COPY_AND_ASSIGN(Thread);
}; // thread

END_NAMESPACE_BASE

#include "std_list"
#include "lock.h"
BEGIN_NAMESPACE_BASE
//////////////////////////////////////////////////////////////////////////
template<typename _Ty>
class ThreadGroup {
public:
  ThreadGroup()
    : m_lstThd()
  {}

  ~ThreadGroup() {

    join_all();
  }

public:
  Thread* create_thread(_Ty* thread, void* context) {

    AutoLock  autoLock(m_lckThdList);
    Thread* thd = new Thread(thread->Name());
    if (NULL == thd) { return NULL; }

    if (FALSE == thd->Start(thread, context)) { delete thd; return NULL; }

    // add list
    m_lstThd.push_back(thd);
    return thd;
  }

  void join_all() {

    AutoLock  autoLock(m_lckThdList);
    for(thd_list_t::iterator it_list = m_lstThd.begin(), end = m_lstThd.end();
      it_list != end;
      ++it_list)
    {
      if (TRUE == (*it_list)->joinable())
        (*it_list)->join();

      delete (*it_list);
    }

    m_lstThd.clear();
  }

private:
  typedef std::list<Thread*>      thd_list_t;
  thd_list_t        m_lstThd;
  Lock              m_lckThdList;

  DISALLOW_COPY_AND_ASSIGN(ThreadGroup);
}; // ThreadGroup

END_NAMESPACE_BASE

#endif // THRED_H_
