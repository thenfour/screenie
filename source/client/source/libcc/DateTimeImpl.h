/*
  Last updated May 27, 2005 carlc

  (c) 2004-2005 Carl Corcoran, carl@ript.net
  http://carl.ript.net/stringformat/
  http://carl.ript.net/wp
  http://mantis.winprog.org/
  http://svn.winprog.org/personal/carlc/stringformat

  All software on this site is provided 'as-is', without any express or
  implied warranty, by its respective authors and owners. In no event will
  the authors be held liable for any damages arising from the use of this
  software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
  claim that you wrote the original software. If you use this software in
  a product, an acknowledgment in the product documentation would be
  appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not
  be misrepresented as being the original software.

  3. This notice may not be removed or altered from any source distribution.
*/

// This is an inline h file for implementation.  Do not #include it.

namespace LibCC
{

  inline TimeSpan::TimeSpan() :
	  m_ticks(0)
  {
  }

  inline TimeSpan::TimeSpan(__int64 ticks) :
	  m_ticks(ticks)
  {
  }

  inline __int64 TimeSpan::GetTicks() const
  {
	  return m_ticks;
  }

  inline TimeSpan::TimeSpan(long hours, long minutes, long seconds)
  {
	  m_ticks = HoursToTicks(hours) + MinutesToTicks(minutes) + SecondsToTicks(seconds);
  }

  inline TimeSpan::TimeSpan(long days, long hours, long minutes, long seconds)
  {
	  m_ticks = DaysToTicks(days) + HoursToTicks(hours) + MinutesToTicks(minutes) + SecondsToTicks(seconds);
  }

  inline TimeSpan::TimeSpan(long days, long hours, long minutes, long seconds, long milliseconds)
  {
	  m_ticks = DaysToTicks(days) + HoursToTicks(hours) +
		  MinutesToTicks(minutes) + SecondsToTicks(seconds) + MillisecondsToTicks(milliseconds);
  }

  inline TimeSpan::TimeSpan(const TimeSpan& r) :
	  m_ticks(r.m_ticks)
  {
  }

  inline void TimeSpan::Add(const TimeSpan& r)
  {
	  m_ticks += r.m_ticks;
  }


  inline void TimeSpan::Negate()
  {
	  m_ticks = -m_ticks;
  }

  inline void TimeSpan::Subtract(const TimeSpan& r)
  {
	  m_ticks -= r.m_ticks;
  }

  inline void TimeSpan::Multiply(double f)
  {
	  m_ticks = static_cast<__int64>(f * static_cast<double>(m_ticks));
  }

  inline long TimeSpan::Compare(const TimeSpan& r) const
  {
	  if(r.m_ticks == m_ticks) return 0;
	  if(m_ticks < r.m_ticks) return -1;
	  return 1;
  }

  inline bool TimeSpan::Equals(const TimeSpan& r) const
  {
	  return m_ticks == r.m_ticks;
  }

  inline bool TimeSpan::LessThan(const TimeSpan& r) const
  {
	  return m_ticks < r.m_ticks;
  }

  inline bool TimeSpan::GreaterThan(const TimeSpan& r) const
  {
	  return m_ticks > r.m_ticks;
  }

  inline bool TimeSpan::LessThanOrEquals(const TimeSpan& r) const
  {
	  return m_ticks <= r.m_ticks;
  }

  inline bool TimeSpan::GreaterThanOrEquals(const TimeSpan& r) const
  {
	  return m_ticks >= r.m_ticks;
  }

  inline long TimeSpan::TicksToSeconds(__int64 t)
  {
	  return static_cast<long>(t / SecondsToTicks(__int64(1)));
  }

  inline __int64 TimeSpan::MillisecondsToTicks(__int64 s)
  {
	  return s * 10000;
  }

  inline __int64 TimeSpan::MillisecondsToTicks(long s)
  {
	  return MillisecondsToTicks(__int64(s));
  }

  inline __int64 TimeSpan::SecondsToTicks(long s)
  {
	  return MillisecondsToTicks(s * 1000);
  }

  inline __int64 TimeSpan::SecondsToTicks(__int64 s)
  {
	  return MillisecondsToTicks(s * 1000);
  }

  inline __int64 TimeSpan::MinutesToTicks(long s)
  {
	  return SecondsToTicks(s * 60);
  }

  inline __int64 TimeSpan::MinutesToTicks(__int64 s)
  {
	  return SecondsToTicks(s * 60);
  }

  inline __int64 TimeSpan::HoursToTicks(long s)
  {
	  return MinutesToTicks(s * 60);
  }

  inline __int64 TimeSpan::HoursToTicks(__int64 s)
  {
	  return MinutesToTicks(s * 60);
  }

  inline __int64 TimeSpan::DaysToTicks(long s)
  {
	  return HoursToTicks(s * 24);
  }

  inline __int64 TimeSpan::DaysToTicks(__int64 s)
  {
	  return HoursToTicks(s * 24);
  }

  inline void TimeSpan::ToFILETIME(FILETIME& ft) const
  {
	  ft.dwHighDateTime = static_cast<DWORD>((m_ticks & 0xFFFFFFFF00000000) >> 32);
	  ft.dwLowDateTime = static_cast<DWORD>(m_ticks & 0xFFFFFFFF);
  }

  inline void TimeSpan::FromFILETIME(const FILETIME& ft)
  {
	  m_ticks = (static_cast<__int64>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
  }


  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  inline DateTime::DateTime() :
	  m_ticks(0),
	  m_kind(DateTimeKind::Unspecified)
  {
    __MarkSystemTimeDirty();
  }

  inline DateTime::DateTime(__int64 ticks, DateTimeKind kind) :
	  m_ticks(ticks),
	  m_kind(kind)
  {
    __MarkSystemTimeDirty();
  }

  inline DateTime::DateTime(const DateTime& r) :
	  m_ticks(r.m_ticks),
	  m_kind(r.m_kind),
    m_stDirty(r.m_stDirty),
    m_st(r.m_st)// not sure if this will work.
  {
  }

  inline DateTime::DateTime(long year, Month month, long day, DateTimeKind kind) :
	  m_kind(kind),
    m_stDirty(false)
  {
    __ClearCachedSystemTime();
    m_st.wYear = static_cast<WORD>(year);
	  m_st.wMonth = static_cast<WORD>(month);
	  m_st.wDay = static_cast<WORD>(day);
	  __FromCachedSystemTime();
  }

  inline DateTime::DateTime(long year, Month month, long day, long hour, long minute, long second, DateTimeKind kind) :
	  m_kind(kind),
    m_stDirty(false)
  {
    __ClearCachedSystemTime();
	  m_st.wYear = static_cast<WORD>(year);
	  m_st.wMonth = static_cast<WORD>(month);
	  m_st.wDay = static_cast<WORD>(day);
	  m_st.wHour = static_cast<WORD>(hour);
	  m_st.wMinute = static_cast<WORD>(minute);
	  m_st.wSecond = static_cast<WORD>(second);
	  __FromCachedSystemTime();
  }

  inline DateTime::DateTime(long year, Month month, long day, long hour, long minute, long second, long millisecond, DateTimeKind kind) :
	  m_kind(kind),
    m_stDirty(false)
  {
    __ClearCachedSystemTime();
	  m_st.wYear = static_cast<WORD>(year);
	  m_st.wMonth = static_cast<WORD>(month);
	  m_st.wDay = static_cast<WORD>(day);
	  m_st.wHour = static_cast<WORD>(hour);
	  m_st.wMinute = static_cast<WORD>(minute);
	  m_st.wSecond = static_cast<WORD>(second);
	  m_st.wMilliseconds = static_cast<WORD>(millisecond);
	  __FromCachedSystemTime();
  }

  inline DateTime::DateTime(const SYSTEMTIME& r, DateTimeKind kind) :
    m_kind(kind),
    m_stDirty(false)
  {
	  FromSystemTime(r);
  }

  inline void DateTime::Add(const TimeSpan& r)
  {
    __MarkSystemTimeDirty();
	  m_ticks += r.GetTicks();
  }

  inline void DateTime::Subtract(const TimeSpan& r)
  {
    __MarkSystemTimeDirty();
	  m_ticks -= r.GetTicks();
  }

  inline TimeSpan DateTime::Subtract(const DateTime& r)
  {
    __MarkSystemTimeDirty();
	  return TimeSpan(m_ticks - r.m_ticks);
  }

  inline void DateTime::AddDays(__int16 d)
  {
    __GetCachedSystemTime().wDay += d;
	  __FromCachedSystemTime();
  }

  inline void DateTime::AddHours(__int16 d)
  {
    __GetCachedSystemTime().wHour += d;
	  __FromCachedSystemTime();
  }

  inline void DateTime::AddMilliseconds(__int16 d)
  {
    __GetCachedSystemTime().wMilliseconds += d;
	  __FromCachedSystemTime();
  }

  inline void DateTime::AddMinutes(__int16 d)
  {
    __GetCachedSystemTime().wMinute += d;
	  __FromCachedSystemTime();
  }

  inline void DateTime::AddMonths(__int16 d)
  {
    // here's the problem... if we are at Jan 31, and i add a month i go to Feb 31... which doesnt exist.
    // what to do?
    __GetCachedSystemTime().wMonth += d;
	  __FromCachedSystemTime();
  }

  inline void DateTime::AddSeconds(__int16 v)
  {
    __GetCachedSystemTime().wSecond += v;
	  __FromCachedSystemTime();
  }

  inline void DateTime::AddTicks(__int64 ticks)
  {
    __MarkSystemTimeDirty();
	  m_ticks += ticks;
  }

  inline void DateTime::AddYears(__int16 d)
  {
    __GetCachedSystemTime().wYear += d;
	  __FromCachedSystemTime();
  }

  inline long DateTime::Compare(const DateTime& s) const
  {
	  if(m_ticks < s.m_ticks)
	  {
		  return -1;
	  }
	  if(m_ticks > s.m_ticks)
	  {
		  return 1;
	  }
	  return 0;
  }

  inline void DateTime::FromFileTime(const FILETIME& ft)
  {
    __MarkSystemTimeDirty();
	  m_ticks = (static_cast<__int64>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
  }

  inline void DateTime::ToFileTime(FILETIME& ft) const
  {
	  ft.dwHighDateTime = static_cast<DWORD>((m_ticks & 0xFFFFFFFF00000000) >> 32);
	  ft.dwLowDateTime = static_cast<DWORD>(m_ticks & 0xFFFFFFFF);
  }

  inline void DateTime::FromSystemTime(const SYSTEMTIME& st)
  {
    memcpy(&m_st, &st, sizeof(st));
    m_stDirty = false;
    __FromCachedSystemTime();
  }

  inline void DateTime::ToSystemTime(SYSTEMTIME& st)
  {
    __GetCachedSystemTime();
    memcpy(&st, &m_st, sizeof(st));
  }

  inline bool DateTime::Equals(const DateTime& s)
  {
	  return m_ticks == s.m_ticks;
  }

  inline bool DateTime::IsDaylightSavingTime()
  {
    // TODO
	  return false;
  }

  inline __int64 DateTime::ToBinary()
  {
	  return m_ticks;
  }

  inline double DateTime::ToOADate()
  {
    // TODO
	  return 0;
  }

  inline DateTime DateTime::ToLocal()
  {
    // TODO
	  return DateTime();
  }

  inline std::string DateTime::ToString()
  {
    // TODO
	  return "";
  }

  inline void DateTime::FromUnixTime(time_t t)
  {
    FILETIME ft;
    LONGLONG ll;
    ll = Int32x32To64(t, 10000000) + 116444736000000000;
    ft.dwLowDateTime = (DWORD)ll;
    ft.dwHighDateTime = (DWORD)(ll >> 32);
    FromFileTime(ft);
  }

  /*
	  1  jan  31
	  2  feb  leap
	  3  mar  31
	  4  apr  30
	  5  may  31
	  6  jun  30
	  7  jul  31
	  8  aug  31
	  9  sep  30
	  10 oct  31
	  11 nov  30
	  12 dec  31
  */
  inline long DateTime::DaysInMonth(long year, Month m)
  {
	  switch(m)
	  {
	  case Month::January:
	  case Month::March:
	  case Month::May:
	  case Month::July:
	  case Month::August:
	  case Month::October:
	  case Month::December:
		  return 31;
	  case Month::April:
	  case Month::June:
	  case Month::September:
	  case Month::November:
		  return 30;
	  case Month::February:
		  return IsLeapYear(year) ? 29 : 28;
	  }

	  return 0;
  }

  inline bool DateTime::IsLeapYear(long y)
  {
	  return ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0);
  }

  inline DateTime DateTime::LocalTime()
  {
    SYSTEMTIME st;
    GetLocalTime(&st);
    return DateTime(st, DateTimeKind::Local);
  }

  inline DateTime DateTime::UTCTime()
  {
    SYSTEMTIME st;
    GetLocalTime(&st);
    return DateTime(st, DateTimeKind::Local);
  }
}

