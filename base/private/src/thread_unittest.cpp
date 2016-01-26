/*

*/

#include "test.h"

#ifdef DO_UNITTEST

#include "thread.h"
#include "waitable_event.h"
#include "lock.h"
USING_NAMESPACE_BASE;

//////////////////////////////////////////////////////////////////////////
class ThreadSample : public Thread {

  // thread
public:
  void ThreadMain(void* context) {

    UNUSED_PARAM(context);
    TRACE(_STR("RUNNING"));

    if (context) {
      // Lock
      Lock* pLock = (Lock*)(context);
      AutoLock autoLock(*pLock);

      sleep(100);
    }

    m_event.Signal();
    sleep(1000);
  }

public:
  ThreadSample() 
    : Thread(_STR("ThreadSample"))
    , m_event(FALSE, FALSE)
    , m_bVal(0)
  {
  }

public:
  WaitableEvent m_event;
  uint32_t m_bVal;
}; // ThreadSample


TEST_BEGIN(base, thread_single) {

  ThreadSample thread;

  ASSERT_EQ(TRUE, thread.Start(NULL));
  thread.m_event.Wait();

  thread.Stop();

} TEST_END

TEST_BEGIN(base, thread_group) {

  ThreadGroup<ThreadSample> threads;
  ThreadSample thread;

  ASSERT_NE(NULL, threads.create_thread(&thread, NULL));
  ASSERT_NE(NULL, threads.create_thread(&thread, NULL));
  
  threads.join_all();

} TEST_END

TEST_BEGIN(base, lock_base) {

  ThreadGroup<ThreadSample> threads;
  ThreadSample thread;

  Lock lock;
  ASSERT_NE(NULL, threads.create_thread(&thread, &lock));
  ASSERT_NE(NULL, threads.create_thread(&thread, &lock));
  
  threads.join_all();

} TEST_END

TEST_BEGIN(base, lock_mock) {
/*
  ThreadGroup<ThreadSample> threads;
  ThreadSample thread;

  LockMock lockMock;
  AutoLock autoLock(lockMock);

  ASSERT_NE(NULL, threads.create_thread(&thread, &lockMock));
  ASSERT_NE(NULL, threads.create_thread(&thread, &lockMock));

  threads.join_all();
*/
} TEST_END

TEST_BEGIN(base, thread) {

  TEST_EXEC(base, lock_base);
  TEST_EXEC(base, lock_mock);

  TEST_EXEC(base, thread_single);
  TEST_EXEC(base, thread_group);

} TEST_END

#endif // DO_UNITTEST
