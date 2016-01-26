/*

*/

#ifndef TIME_UTIL_H_
#define TIME_UTIL_H_

#include "base.h"
#include "base_export.h"

BEGIN_NAMESPACE_BASE

typedef uint64_t                Time;
//////////////////////////////////////////////////////////////////////////
class BASE_EXPORT micro_time {
public:
  static uint32_t size() { return sizeof(uint64_t); }
  static uint64_t tick();

public:
  static const uint64_t time() { return tick() - m_gStartTime; }

public:
  static const uint64_t to_millisecond(const uint64_t tick) { return tick / 1000; }
  static const uint64_t to_second(const uint64_t tick) { return tick / 1000000; }

  // 20 len
  enum { TIME_FORAT_LEN = 11, TIME_FORAT_WITH_YMD_LEN = 20 };
  static const char_t* format_with_ymd(char_t* dest, const uint64_t time); // millisecond 
  static const char_t* format(char_t* dest, const uint64_t time); // millisecond 

public:
  static void set_start(const uint64_t tick);
  static const uint64_t time_start() { return m_gStartTime; }

public:
  static const uint64_t m_gcStartTime;

private:
  static uint64_t m_gStartTime;
  static uint64_t m_gStartTime_std;

  friend class DayTime;
}; // micro_time

//////////////////////////////////////////////////////////////////////////
// 24hours
class BASE_EXPORT DayTime {
public:
  static uint32_t now(); // sec time(NULL)
  static uint32_t to_localtime(uint32_t year, uint32_t mon, uint32_t day, uint32_t hour, uint32_t min, uint32_t sec);
  static bool_t get_localtime(uint32_t* year, uint32_t* mon, uint32_t* day, uint32_t* hour, uint32_t* min, uint32_t* sec, uint32_t);

  static uint32_t to_string(uint32_t time);           // 0x....  => 114532
  static uint32_t to_string(uint64_t time);           // 0x....  => 114532
  static uint32_t to_time(uint32_t time_sec);         //

  static uint32_t day_sec(uint32_t time_sec);   // sec on today.

public:
  enum { START = 0, END_SEC = 86399, END = 235959, INVALID = 240000 };
}; // DayTime


//////////////////////////////////////////////////////////////////////////
class BASE_EXPORT five_year_time {

public:
  uint32_t m_time;

public:
  five_year_time(void* data);

public:
  time_t to_time()    { return (m_time >> MSEC_BITS) + m_gStartTime; }
  uint32_t microsec() { return m_time & MARK_MICROSEC; }

public:
  static uint32_t size() { return sizeof(uint32_t); }
  static uint32_t time() { return 0; }

public:
  static uint32_t is_valid() { return time(); }
  static void set_time(uint32_t time) { m_gStartTime = time; }
  static uint32_t m_gStartTime;

private:
  enum {MAX_USE_BITS  = 32, TIME_BITS     = 28, MSEC_BITS     = MAX_USE_BITS - TIME_BITS, MAX_TIME = 1 << TIME_BITS};
  enum {MARK_MICROSEC = 1 << TIME_BITS};
}; // five_year_time

// 5Y = (24 * 60 * 60 = 86400) * 365 * 5 = 157680000
// 4294967295 / 157680000 = 27.2385

//////////////////////////////////////////////////////////////////////////
class TimeControl {
public:
  TimeControl(uint32_t nTimeOut)
    : m_nTimeOut(nTimeOut)
    , m_nTimeLast(DayTime::now())
  {}

public:
  inline uint32_t GetIdleTime(uint32_t now) {
    if(now > m_nTimeLast) {
      uint32_t nIdleTime = now - m_nTimeLast;
      if (nIdleTime > m_nTimeOut) { m_nTimeLast = now; return 0; }
      return nIdleTime;
    }
    return m_nTimeOut;
  }

  inline void UpdateTime() { m_nTimeLast = DayTime::now(); }
  inline void UpdateTime(uint32_t time) { m_nTimeLast = time; }

  inline void SetTimeOut(uint32_t nTimeOut) { m_nTimeOut = nTimeOut; }
  inline void SetIdleMultiple(uint32_t mul) { m_nTimeLast += mul * m_nTimeOut; }

  inline void MakeIdle() { m_nTimeOut = 0; }

  inline void MakeIdleOnce() {
    uint32_t now = DayTime::now();
    if(now > m_nTimeLast) {

      uint32_t nIdleTime = now - m_nTimeLast;
      if (nIdleTime > m_nTimeOut) { return ; }
    }
    m_nTimeLast = now - m_nTimeOut;
  }

public:
  inline uint32_t GetTimeOut() const { return m_nTimeOut; }
  inline uint32_t GetLastTime() const { return m_nTimeLast; }

private:
  uint32_t m_nTimeLast;
  uint32_t m_nTimeOut;
};

END_NAMESPACE_BASE

#endif // TIME_UTIL_H_
