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
/*
  ticks 
  A date and time expressed in 100-nanosecond units. 
*/

#pragma once

#include "cctypes.h"
// for COM types
#include <comutil.h>



namespace LibCC
{
  scope_enum(DateTimeKind)
  {
	  Unspecified,
	  UTC,
	  Local
	  end_scope_enum(DateTimeKind);
  };

  scope_enum(Month)
  {
	  January = 1,
	  February = 2,
	  March = 3,
	  April = 4,
	  May = 5,
	  June = 6,
	  July = 7,
	  August = 8,
	  September = 9,
	  October = 10,
	  November = 11,
	  December = 12
	  end_scope_enum(Month);
  };

  scope_enum(Weekday)
  {
	  Sunday = 1,
	  Monday = 2,
	  Tuesday = 3,
	  Wednesday = 4,
	  Thursday = 5,
	  Friday = 6,
	  Saturday = 7
	  end_scope_enum(Weekday);
  };


  class TimeSpan
  {
  public:
	  TimeSpan();
	  TimeSpan(__int64 ticks);
	  TimeSpan(long hours, long minutes, long seconds);
	  TimeSpan(long days, long hours, long minutes, long seconds);
	  TimeSpan(long days, long hours, long minutes, long seconds, long milliseconds);
	  TimeSpan(const TimeSpan& r);

	  void Add(const TimeSpan& r);
	  void Negate();
	  void Subtract(const TimeSpan& r);
	  void Multiply(double f);
	  void Multiply(long x);
	  void Divide(double f);
	  void Divide(long x);

	  long Compare(const TimeSpan& r) const;
	  bool Equals(const TimeSpan& r) const;
	  bool LessThan(const TimeSpan& r) const;
	  bool GreaterThan(const TimeSpan& r) const;
	  bool LessThanOrEquals(const TimeSpan& r) const;
	  bool GreaterThanOrEquals(const TimeSpan& r) const;

	  __int64 GetTicks() const;
	  void ToFILETIME(FILETIME& ft) const;
	  void FromFILETIME(const FILETIME& ft);

	  static long TicksToSeconds(__int64 t);
	  static __int64 MillisecondsToTicks(long s);
	  static __int64 MillisecondsToTicks(__int64 s);
	  static __int64 SecondsToTicks(long s);
	  static __int64 SecondsToTicks(__int64 s);
	  static __int64 MinutesToTicks(long s);
	  static __int64 MinutesToTicks(__int64 s);
	  static __int64 HoursToTicks(long s);
	  static __int64 HoursToTicks(__int64 s);
	  static __int64 DaysToTicks(long s);
	  static __int64 DaysToTicks(__int64 s);
  private:
	  __int64 m_ticks;
  };


  class DateTime
  {
  public:
	  DateTime();// uses local time NOW
	  DateTime(__int64 ticks, DateTimeKind kind = DateTimeKind::Unspecified);
	  DateTime(const DateTime& r);
	  DateTime(const SYSTEMTIME& r, DateTimeKind kind = DateTimeKind::Unspecified);
	  DateTime(long year, Month month, long day, DateTimeKind kind = DateTimeKind::Unspecified);
	  DateTime(long year, Month month, long day, long hour, long minute, long second, DateTimeKind kind = DateTimeKind::Unspecified);
	  DateTime(long year, Month month, long day, long hour, long minute, long second, long millisecond, DateTimeKind kind = DateTimeKind::Unspecified);

	  void Assign(__int64 ticks, DateTimeKind kind);

	  void Add(const TimeSpan& r);
	  void Subtract(const TimeSpan& r);
	  TimeSpan Subtract(const DateTime& r);
	  void AddDays(__int16 d);
	  void AddHours(__int16 d);
	  void AddMilliseconds(__int16 d);
	  void AddMinutes(__int16 d);
	  void AddMonths(__int16 d);
	  void AddSeconds(__int16 v);
	  void AddTicks(__int64 ticks);
	  void AddYears(__int16 d);

    long GetYear() { return __GetCachedSystemTime().wYear; };
    Month GetMonth() { return Month(__GetCachedSystemTime().wMonth); };
    long GetDay() { return __GetCachedSystemTime().wDay; };
    long GetHour() { return __GetCachedSystemTime().wHour; };
    long GetMinute() { return __GetCachedSystemTime().wMinute; };
    long GetSecond() { return __GetCachedSystemTime().wSecond; };
    long GetMillisecond() { return __GetCachedSystemTime().wMilliseconds; };

	  long Compare(const DateTime& s) const;
	  bool Equals(const DateTime& s);
	  bool IsDaylightSavingTime();
	  __int64 ToBinary();
	  double ToOADate();
	  DateTime ToLocal();// adjust for daylight savings and time zone
	  DateTime ToUTC();// adjust for daylight savings and time zone
	  std::string ToString();

    void FromUnixTime(time_t t);
    void ToUnixTime(time_t&) const;
    time_t ToUnixTime() const;

    void FromVariant(VARIANT& v);
    void ToVariant(_variant_t&) const;
    _variant_t ToVariant() const;

	  void FromFileTime(const FILETIME& ft);
	  void ToFileTime(FILETIME& ft) const;
	  FILETIME ToFileTime() const;

	  void FromSystemTime(const SYSTEMTIME& ft);
	  void ToSystemTime(SYSTEMTIME& ft);
	  SYSTEMTIME ToSystemTime();

	  static long DaysInMonth(long year, Month m);
	  static bool IsLeapYear(long y);

    static DateTime LocalTime();
    static DateTime UTCTime();

  private:
	  __int64 m_ticks;
	  DateTimeKind m_kind;

    void __FromCachedSystemTime()
    {
	    FILETIME ft;
	    SystemTimeToFileTime(&m_st, &ft);
	    FromFileTime(ft);
      m_stDirty = false;
    }

    void __ClearCachedSystemTime()
    {
      memset(&m_st, 0, sizeof(m_st));
    }
    void __MarkSystemTimeDirty()
    {
      m_stDirty = true;
    }
    SYSTEMTIME& __GetCachedSystemTime()
    {
      if(m_stDirty)
      {
	      FILETIME ft;
	      ToFileTime(ft);
	      FileTimeToSystemTime(&ft, &m_st);
        m_stDirty = false;
      }
      return m_st;
    }

    SYSTEMTIME m_st;
    bool m_stDirty;// true if m_st is set;
  };

}

#include "DateTimeImpl.h"
