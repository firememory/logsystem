/*

*/
#include "time_util.h"

BEGIN_NAMESPACE_BASE

#include <time.h>

#if defined(PLATFORM_API_WINDOWS)
# define __WIN__
# include <windows.h>
#else
# include <syswait.h>
#endif // PLATFORM_API_WINDOWS

/* mysql code

*/

/*
That time is probably representing 100 nanosecond units since Jan 1. 1601. 
There's 116444736000000000 100ns between 1601 and 1970.

*/

typedef uint64_t      ulonglong;

static ulonglong ___get_micro_time() {

#if defined(__WIN__)
  ulonglong newtime;
  GetSystemTimeAsFileTime((FILETIME*)&newtime);
  return (newtime/10);
#elif defined(HAVE_GETHRTIME)
  return gethrtime()/1000;
#else
  ulonglong newtime;
  struct timeval t;
  /*
  The following loop is here because gettimeofday may fail on some systems
  */
  while (gettimeofday(&t, NULL) != 0)
  {}
  newtime= (ulonglong)t.tv_sec * 1000000 + t.tv_usec;
  return newtime;
#endif  /* defined(__WIN__) */
}
/*
ulonglong __get_micro_time() {

#if defined(PLATFORM_API_WINDOWS)
  return ___get_micro_time();
#else // linux
  return ___get_micro_time() + 11644473600000000;
#endif 
}
*/

//////////////////////////////////////////////////////////////////////////
// micro_time
const uint64_t micro_time::m_gcStartTime = 0x002E3E3A32343E00; // 20130621 14:10
uint64_t micro_time::m_gStartTime = 0x002E3E3A32343E00; // 20130621 14:10
uint64_t micro_time::m_gStartTime_std = 0x51C3EE38; // 1371795000

uint64_t micro_time::tick() {

#if defined(PLATFORM_API_WINDOWS)
  return ___get_micro_time();
#else // linux
  return ___get_micro_time() + 11644473600000000;
#endif 
}

void micro_time::set_start(const uint64_t tick) {

  uint64_t tick__ = (tick / 1000000) * 1000000;

  if (tick__ > m_gStartTime) {
    m_gStartTime_std += to_second(tick__ - m_gStartTime);
  }
  else {
    m_gStartTime_std -= to_second(m_gStartTime - tick__);
  }

  m_gStartTime = tick__;
}


#define LOCALTIME(tm, time) localtime_s(&(tm), &(time))
// millisecond 
const char_t* micro_time::format_with_ymd(char_t* buffer, uint64_t time) {
  
  const char_t kStartNumber = (char_t)('0');
  //
  uint64_t milli_sec = to_millisecond(time);
  uint64_t sec = milli_sec / 1000;
  milli_sec = milli_sec % 1000;
  time_t now = m_gStartTime_std + sec;

  struct tm today;
  LOCALTIME(today, now);
  // XXX
  int year = today.tm_year + 1900;
  int month = today.tm_mon + 1;

  /* store thousands of year */
  buffer[0] = (char_t)(year             / 1000 + kStartNumber);
  /* store hundreds of year */
  buffer[1] = (char_t)((year/ 100)      % 10 + kStartNumber);
  /* store tens of year */
  buffer[2] = (char_t)((year/ 10)       % 10 + kStartNumber);
  /* store units of year */
  buffer[3] = (char_t)(year             % 10 + kStartNumber);
  /* store tens of month */
  buffer[4] = (char_t)(month            / 10 + kStartNumber);
  /* store units of month */
  buffer[5] = (char_t)(month            % 10 + kStartNumber);
  /* store tens of day */
  buffer[6] = (char_t)(today.tm_mday    / 10 + kStartNumber);
  /* store units of day */
  buffer[7] = (char_t)(today.tm_mday    % 10 + kStartNumber);

  buffer[8] = (char_t)('-');

  /* store tens of hour */
  buffer[9] = (char_t)(today.tm_hour    / 10 + kStartNumber);
  /* store units of hour */
  buffer[10] = (char_t)(today.tm_hour   % 10 + kStartNumber);
  /* store tens of minute */
  buffer[11]= (char_t)(today.tm_min     / 10 + kStartNumber);
  /* store units of minute */
  buffer[12]= (char_t)(today.tm_min     % 10 + kStartNumber);
  /* store tens of second */
  buffer[13]= (char_t)(today.tm_sec     / 10 + kStartNumber);
  /* store units of second */
  buffer[14]= (char_t)(today.tm_sec     % 10 + kStartNumber);

  // millisecond
  buffer[15]= (char_t)('.');
  buffer[16]= (char_t)(milli_sec        / 1000 + kStartNumber);
  buffer[17]= (char_t)((milli_sec/ 100) % 10 + kStartNumber);
  buffer[18]= (char_t)(milli_sec        % 10 + kStartNumber);

  // XXX
  buffer[19] = 0;
  return buffer;
}

const char_t* micro_time::format(char_t* buffer, uint64_t time) {

  const char_t kStartNumber = (char_t)('0');
  //
  uint64_t milli_sec = to_millisecond(time);
  uint64_t sec = milli_sec / 1000;
  milli_sec = milli_sec % 1000;
  time_t now = m_gStartTime_std + sec;

  struct tm today;
  LOCALTIME(today, now);

  /* store tens of hour */
  buffer[0] = (char_t)(today.tm_hour    / 10 + kStartNumber);
  /* store units of hour */
  buffer[1] = (char_t)(today.tm_hour   % 10 + kStartNumber);
  /* store tens of minute */
  buffer[2]= (char_t)(today.tm_min     / 10 + kStartNumber);
  /* store units of minute */
  buffer[3]= (char_t)(today.tm_min     % 10 + kStartNumber);
  /* store tens of second */
  buffer[4]= (char_t)(today.tm_sec     / 10 + kStartNumber);
  /* store units of second */
  buffer[5]= (char_t)(today.tm_sec     % 10 + kStartNumber);

  // millisecond
  buffer[6]= (char_t)('.');
  buffer[7]= (char_t)(milli_sec        / 1000 + kStartNumber);
  buffer[8]= (char_t)((milli_sec/ 100) % 10 + kStartNumber);
  buffer[9]= (char_t)(milli_sec        % 10 + kStartNumber);

  // XXX
  buffer[10] = 0;
  return buffer;
}

//////////////////////////////////////////////////////////////////////////
// DayTime
uint32_t DayTime::now() {
  return (uint32_t)time(NULL);
}


#define MKTIME    mktime

uint32_t DayTime::to_localtime(uint32_t year, uint32_t mon, uint32_t day, uint32_t hour, uint32_t min, uint32_t sec) {

  struct tm now;

  now.tm_year = year - 1900;
  now.tm_mon  = mon - 1;
  now.tm_mday = day;
  now.tm_hour = hour;
  now.tm_min  = min;
  now.tm_sec  = sec;

  return (uint32_t)MKTIME(&now);
}

bool_t DayTime::get_localtime(uint32_t* year, uint32_t* mon, uint32_t* day
                              , uint32_t* hour, uint32_t* min, uint32_t* sec
                              , uint32_t srcTime) {

  /* */
  time_t now = srcTime;
  struct tm today;
  LOCALTIME(today, now);
  
  if (year) { *(year) = today.tm_year + 1900; }
  if (mon)  { *(mon)  = today.tm_mon + 1; }
  if (day)  { *(day)  = today.tm_mday; }

  if (hour) { *(hour) = today.tm_hour; }
  if (min)  { *(min)  = today.tm_min; }
  if (sec)  { *(sec)  = today.tm_sec; }

  return TRUE;
}

uint32_t DayTime::to_string(uint32_t sec) {

  //time_t now = micro_time::m_gStartTime_std + sec;
  time_t now = sec;
  struct tm today;
  LOCALTIME(today, now);

  return today.tm_hour * 10000 
    + today.tm_min * 100
    + today.tm_sec;
}

uint32_t DayTime::to_string(uint64_t time) {

  uint64_t milli_sec = micro_time::to_millisecond(time);
  uint64_t sec = milli_sec / 1000;
  time_t now = micro_time::m_gStartTime_std + sec;

  struct tm today;
  LOCALTIME(today, now);

  return today.tm_hour * 10000 
    + today.tm_min * 100
    + today.tm_sec;
}

uint32_t DayTime::to_time(uint32_t time_sec) {

  uint32_t hour = time_sec / 10000;
  uint32_t min  = (time_sec / 100) % 100;
  uint32_t sec  = time_sec % 100;
  return hour * 60 * 60
    + min * 60
    + sec;
}

// 
uint32_t DayTime::day_sec(uint32_t time_sec) {

  time_t now = (time_t)time_sec;
  struct tm today;
  LOCALTIME(today, now);

  return today.tm_hour * 60 * 60
    + today.tm_min * 60
    + today.tm_sec;
}

//////////////////////////////////////////////////////////////////////////
// five_year_time

uint32_t five_year_time::m_gStartTime = 0;

END_NAMESPACE_BASE
