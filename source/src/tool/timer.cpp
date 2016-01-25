#include "timer.h"
#include <chrono>

#if !defined(WIN32) || (defined(WIN32) && (_MSC_VER >= 1800) ) 
// support for stl chrono in high resolution from VS 2013 onwards:

typedef std::chrono::high_resolution_clock high_resolution_clock;

#else
#include <windows.h>
// we want more than 0.5ms accuracy
struct HighResClock
{
  typedef long long                               rep;
  typedef std::nano                               period;
  typedef std::chrono::duration<rep, period>      duration;
  typedef std::chrono::time_point<HighResClock>   time_point;
  static const bool is_steady = true;

  static time_point now();
};

// static
namespace
{
  const long long g_Frequency = []() -> long long
  {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return frequency.QuadPart;
  }();
}

HighResClock::time_point HighResClock::now()
{
  LARGE_INTEGER count;
  QueryPerformanceCounter(&count);
  return time_point(duration(count.QuadPart * static_cast<rep>(period::den) / g_Frequency));
}
typedef HighResClock high_resolution_clock;
#endif

struct Timer::Data {
  high_resolution_clock::time_point m_start;
  Data()
    : m_start(high_resolution_clock::now())
  {
  }
};


Timer::Timer()
: d(0)
{
  d = new Data;
}

Timer::~Timer()
{
  delete d;
}

void
Timer::restart()
{
  d->m_start = high_resolution_clock::now();
}

double
Timer::getElapsedTimeMs() const
{
  return 1.0*std::chrono::duration_cast<std::chrono::nanoseconds>(high_resolution_clock::now() - d->m_start).count() / (1000 * 1000);
}

/*
this is the default implementation of the timer class -- which fails on VS2012 as high_resolution_clock is not using QueryPerformanceCounter().
Once the shift to VS2013 is done, this implementation can be used.
class LRTimer {
private:
typedef std::chrono::high_resolution_clock high_resolution_clock;
high_resolution_clock::time_point m_start;

public:
//! default constructor starts immediately.
LRTimer()
: m_start(high_resolution_clock::now())
{
}
void restart()
{
m_start = high_resolution_clock::now();
}
//! gets the elapsed time since start in milliseconds.
double getElapsedTimeMs() const
{
return 1.0*std::chrono::duration_cast<std::chrono::nanoseconds>(high_resolution_clock::now() - m_start).count()/(1000*1000);
}
}; // end class Timer
*/
