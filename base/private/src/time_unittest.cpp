/*

*/

#include "test.h"

#ifdef DO_UNITTEST

#include "time_util.h"
#include "thread.h"

USING_NAMESPACE_BASE;

TEST_BEGIN(base, micro_time) {

  uint64_t tick = micro_time::tick();
  tick = micro_time::tick();

  tick = micro_time::time();

  uint64_t milli_sec = micro_time::to_millisecond(micro_time::time());
  milli_sec = micro_time::to_millisecond(micro_time::time());

  uint64_t sec = micro_time::to_second(micro_time::time());
  sec = micro_time::to_second(micro_time::time());

  char_t strTime[micro_time::TIME_FORAT_LEN];
  TRACE(micro_time::format(strTime, micro_time::time()));
  Thread::sleep(1);
  TRACE(micro_time::format(strTime, micro_time::time()));
  Thread::sleep(1);
  TRACE(micro_time::format(strTime, micro_time::time()));
  Thread::sleep(1);
  TRACE(micro_time::format(strTime, micro_time::time()));
  Thread::sleep(50);
  TRACE(micro_time::format(strTime, micro_time::time()));
  Thread::sleep(1000);
  micro_time::set_start(micro_time::tick());
  TRACE(micro_time::format(strTime, micro_time::time()));

  micro_time::set_start(micro_time::m_gcStartTime);
  TRACE(micro_time::format(strTime, micro_time::time()));

} TEST_END


TEST_BEGIN(base, day_time) {

  uint32_t now_time = DayTime::now();
  uint32_t local_time = DayTime::to_localtime(2013, 7, 26, 11, 25, 50);

  TRACE("%u", now_time);
  TRACE("%u", local_time);

} TEST_END

TEST_BEGIN(base, five_year_time) {

} TEST_END


#include <time.h>

TEST_BEGIN(base, std_time) {

  char tmpbuf[128];
  _strtime( tmpbuf );
  TRACE( "OS time:\t\t\t\t%s\n", tmpbuf );

  time_t now_time = time(NULL);
  TRACE("%u", now_time);

} TEST_END

#include "file_util.h"
USING_NAMESPACE;

TEST_BEGIN(base, file_time) {

  PlatformFileInfo fileInfo;
  AutoReleaseFile autoRelFile(OpenFile("z:/test.txt", strAttrOpenRead));
  if (autoRelFile) {
    GetPlatformFileInfo(autoRelFile, &fileInfo);
  }

  Time now = micro_time::tick();
  now = now;

} TEST_END


TEST_BEGIN(base, time) {

  TEST_EXEC(base, std_time);
  TEST_EXEC(base, micro_time);
  TEST_EXEC(base, five_year_time);

  TEST_EXEC(base, day_time);

  TEST_EXEC(base, file_time);

} TEST_END

#endif // DO_UNITTEST
