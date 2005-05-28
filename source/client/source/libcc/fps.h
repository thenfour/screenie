/*
  Jun 30 2004 carlc
  class to automate doing frames per second functionality.

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

  FPS fps;
  
  while(each frame)
  {
    // do your frame stuff...

    fps.OnFrame();
    // display the fps
    display(fps.GetFPSString());
  }

  call SetRecalcInterval() if you want to refresh the fps less frequently.
*/

#pragma once

#include <windows.h>
#include "ccstr.h"


namespace LibCC
{
  class FPS
  {
  public:
    FPS() :
      m_fps(0),
      m_lasttick(0),
      m_interval(0),
      m_frames(0),
      m_totallasttick(0),
      m_totalframes(0)
    {
      LARGE_INTEGER lifreq;
      QueryPerformanceFrequency(&lifreq);
      m_freq = (double)lifreq.QuadPart;
    }

    void SetRecalcInterval(double secs)
    {
      m_interval = (LONGLONG)(secs * m_freq);
    }

    inline void OnFrame()
    {
      LONGLONG ct = GetCurrentTick();
      LONGLONG delta = ct - m_lasttick;
      m_frames ++;
      m_totalframes ++;

      if(delta > m_interval)
      {
        // recalc fps and reset m_frames
        m_fps = (double)m_frames / TicksToSeconds(delta);
        m_frames = 0;
        m_lasttick = ct;
      }
    }

    inline void ResetTotal()
    {
      m_totalframes = 0;
      m_totallasttick = GetCurrentTick();
    }

    inline double GetAvgFPS() const
    {
      LONGLONG ct = GetCurrentTick();
      LONGLONG delta = ct - m_totallasttick;
      return (double)m_totalframes / TicksToSeconds(delta);
    }

    inline std::string GetAvgFPSString() const
    {
      return Format("%").d<4>(GetAvgFPS()).Str();
    }

    inline double GetFPS() const
    {
      return m_fps;
    }

    inline std::string GetFPSString() const
    {
      return Format("%").d<4>(m_fps).Str();
    }

  private:

    inline double TicksToSeconds(LONGLONG n) const
    {
      return (double)n / m_freq;
    }

    inline static LONGLONG GetCurrentTick()
    {
      LARGE_INTEGER li;
      QueryPerformanceCounter(&li);
      return li.QuadPart;
    }
    double m_fps;
    double m_freq;// units per second
    long m_frames;// # of frames since last recalc
    LONGLONG m_interval;// how many units until we refresh m_fps
    LONGLONG m_lasttick;

    LONGLONG m_totallasttick;
    LONGLONG m_totalframes;
  };




  class Timer
  {
  public:
    Timer() :
      m_lasttick(0),
      m_lasttick2(0)
    {
      LARGE_INTEGER lifreq;
      QueryPerformanceFrequency(&lifreq);
      m_freq = (double)lifreq.QuadPart;
    }

    // call this to "tick" the timer... the time between the previous tick and this one is now stored.
    inline Tick()
    {
      LARGE_INTEGER li;
      QueryPerformanceCounter(&li);
      m_lasttick2 = m_lasttick;
      m_lasttick = li.QuadPart;
      m_delta = m_lasttick - m_lasttick2;
    }

    inline double GetLastDelta() const
    {
      return TicksToSeconds(m_delta);
    }

    inline std::string GetLastDeltaString() const
    {
      return Format("%").d<4>(GetLastDelta()).Str();
    }

  private:

    inline double TicksToSeconds(LONGLONG n) const
    {
      return (double)n / m_freq;
    }

    double m_freq;// units per second
    LONGLONG m_lasttick;// 1 tick ago
    LONGLONG m_lasttick2;// 2 ticks ago
    LONGLONG m_delta;// diff between lasttick and lasttick2
  };
}