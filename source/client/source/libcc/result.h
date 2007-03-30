/*
  LibCC Release "March 9, 2007"
  Result Module
  (c) 2004-2007 Carl Corcoran, carlco@gmail.com
  Documentation: http://wiki.winprog.org/wiki/LibCC

	== License:

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

#pragma once

#ifdef WIN32

#include <windows.h>
#include "StringUtil.h"

namespace LibCC
{
  /*
    TODO:
    - find ways to maybe not use the text if the caller doesnt need it
    - find ways to make the text perform better

    Result MyFunc()
    {
      return false;
    }

    Locale l;
    Result MyFunc()
    {
      Result r;
      if(!SomeOtherFunc())
      {
        r.Fail(Format(l, IDS_FILENOTFOUND).s(FileName));
      }
      return r;
    }

    Result x = MyFunc();
    if(!x)// or if(x.Failed())
    {
      Log.Write(x.GetText());
    }

    really cool HRESULT wrapper
  */
  template<typename TChar>
  class ResultX
  {
  public:
    typedef TChar Char;
    typedef std::basic_string<Char> String;
    typedef ResultX<Char> This;
    typedef FormatX<Char, std::char_traits<Char>, std::allocator<Char> > Format_T;

  private:
    // don't allow ambiguous calls
    explicit ResultX(bool, ...) { }
    void Assign(bool, ...) { }
    This& operator =(bool) { return *this; }

  public:
    // ---------------------------------Statics
    static This FromWin32(int error)
    {
      return This(HRESULT_FROM_WIN32(error), Format_T("Error #%: %").i(error).gle(error).Str());
    }
    static This FromGetLastError()
    {
      return FromWin32(GetLastError());
    }
    static This Success()
    {
      return This(S_OK);
    }
    static This Failure(HRESULT hr = E_FAIL)
    {
      return This(hr);
    }

    // ---------------------------------Constructors
    ResultX() :
      m_hr(S_OK)
    {
    }
    ResultX(const This& r) :
      m_hr(r.m_hr),
      m_String(r.m_String)
    {
    }
    ResultX(HRESULT hr) :
      m_hr(hr)
    {
    }
    //ResultX(bool b) :
    //  m_hr(b ? S_OK : E_FAIL)
    //{
    //}
    ResultX(HRESULT hr, const Char* text) :
      m_hr(hr)
    {
			ConvertString(text, m_String);
    }
    ResultX(HRESULT hr, const String& text) :
      m_hr(hr)
    {
			ConvertString(text, m_String);
    }

    // --------------------------------- Assignment
    void Assign(HRESULT hr)
    {
      m_hr = hr;
      m_String.clear();
    }
    void Assign(HRESULT hr, const Char* text)
    {
      m_hr = hr;
      m_String.assign(text);
    }
    void Assign(HRESULT hr, const String& text)
    {
      m_hr = hr;
      m_String.assign(text);
    }
    This& operator =(const This& r)
    {
      m_hr = r.m_hr;
      m_String = r.m_String;
      return *this;
    }
    This& operator =(HRESULT hr)
    {
      m_hr = hr;
      m_String.clear();
      return *this;
    }

    // --------------------------------- More Assignment
    This& Fail()
    {
      Assign(E_FAIL);
      return *this;
    }

    // r.Fail("the shit dont work")
    // r.Fail(Format("This shit dont work: %").qs(FileName));
    This& Fail(const String& s)
    {
      Assign(E_FAIL, s);
      return *this;
    }
    This& Fail(const Char* s)
    {
      Assign(E_FAIL, s);
      return *this;
    }

    This& Succeed()
    {
      Assign(S_OK);
      return *this;
    }

    This& Succeed(const String& s)
    {
      Assign(S_OK, s);
      return *this;
    }
    This& Succeed(const Char* s)
    {
      Assign(S_OK, s);
      return *this;
    }

    This& Prepend(const String& s)
		{
			m_String = s + m_String;
      return *this;
		}

    // --------------------------------- Query
    //operator bool() const
    //{
    //  return Succeeded();
    //}

    bool operator !() const
    {
      return Failed();
    }

    bool Succeeded() const
    {
      return SUCCEEDED(m_hr);
    }

    bool Failed() const
    {
      return FAILED(m_hr);
    }

    HRESULT hr() const
    {
      return m_hr;
    }

    const String& str() const
    {
      return m_String;
    }

		bool operator ==(const This& rhs)
		{
			return rhs.m_hr == m_hr;// ignore string description for comparisons.
		}

  private:
    HRESULT m_hr;
    String m_String;
  };
  typedef ResultX<wchar_t> ResultW;
  typedef ResultX<char> ResultA;
  typedef ResultX<TCHAR> Result;
}



#endif