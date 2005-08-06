

#pragma once


#include "ccstr.h"


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

    explicit ResultX(HRESULT hr) :
      m_hr(hr)
    {
    }
    ResultX(HRESULT hr, const String& text) :
      m_hr(hr)
    {
      StringCopy(m_String, text);
    }
    ResultX(HRESULT hr, const Format_T& text) :
      m_hr(hr)
    {
      StringCopy(m_String, text.Str());
    }

    explicit ResultX(bool b) :
      m_hr(b ? S_OK : E_FAIL)
    {
    }
    ResultX(bool b, const String& text) :
      m_hr(b ? S_OK : E_FAIL)
    {
      StringCopy(m_String, text);
    }
    ResultX(bool b, const Format_T& text) :
      m_hr(b ? S_OK : E_FAIL)
    {
      StringCopy(m_String, text.Str());
    }

    // --------------------------------- Assignment
    This& Assign(HRESULT hr)
    {
      m_hr = hr;
      m_String.clear();
      return *this;
    }
    This& Assign(HRESULT hr, const String& text)
    {
      m_hr = hr;
      StringCopy(m_String, text);
      return *this;
    }
    This& Assign(HRESULT hr, const Format_T& text)
    {
      return Assign(hr, text.Str());
    }

    This& Assign(bool b)
    {
      return Assign(b ? S_OK : E_FAIL);
    }
    This& Assign(bool b, const String& text)
    {
      return Assign(b ? S_OK : E_FAIL, text);
    }
    This& Assign(bool b, const Format_T& text)
    {
      return Assign(b ? S_OK : E_FAIL, text);
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
      return Assign(E_FAIL);
    }
    // r.Fail("the shit dont work")
    // r.Fail(Format("This shit dont work: %").qs(FileName));
    This& Fail(const String& s)
    {
      return Assign(E_FAIL, s);
    }
    This& Fail(const Format_T& s)
    {
      return Fail(s.Str());
    }

    This& Succeed()
    {
      return Assign(S_OK);
    }
    This& Succeed(const String& s)
    {
      return Assign(S_OK, s);
    }
    This& Succeed(const Format_T& s)
    {
      return Assign(s.Str());
    }

    This& Prepend(const String& s)
		{
			m_String = s + m_String;
      return *this;
		}

    This& Prepend(const Format_T& s)
		{
      return Prepend(s.Str());
		}

    // --------------------------------- Query
		class BoolLike
		{
      ~BoolLike() {}
    public:
      static BoolLike* True()
      {
        BoolLike true_;
        BoolLike* ret = &true_;
        return ret;
      }
      static BoolLike* False()
      {
        return 0;
      }
      static BoolLike* FromBool(bool b)
      {
        return b ? True() : False();
      }
		};

    operator BoolLike*() const
    {
      return BoolLike::FromBool(Succeeded());
    }

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

  private:
    HRESULT m_hr;
    String m_String;
  };
  typedef ResultX<wchar_t> ResultW;
  typedef ResultX<char> ResultA;
  typedef ResultX<TCHAR> Result;
}



