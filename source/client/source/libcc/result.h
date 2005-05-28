

#pragma once


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
    ResultX(bool b) :
      m_hr(b ? S_OK : E_FAIL)
    {
    }
    ResultX(HRESULT hr, const Char* text) :
      m_hr(hr)
    {
      StringCopy(m_String, text);
    }
    ResultX(HRESULT hr, const String& text) :
      m_hr(hr)
    {
      StringCopy(m_String, text);
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
      StringCopy(m_String, text);
    }
    void Assign(HRESULT hr, const String& text)
    {
      m_hr = hr;
      StringCopy(m_String, text);
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
    void Fail()
    {
      Assign(E_FAIL);
    }

    // r.Fail("the shit dont work")
    // r.Fail(Format("This shit dont work: %").qs(FileName));
    void Fail(const String& s)
    {
      Assign(E_FAIL, s);
    }
    void Fail(const Char* s)
    {
      Assign(E_FAIL, s);
    }

    void Succeed()
    {
      Assign(S_OK);
    }

    void Succeed(const String& s)
    {
      Assign(S_OK, s);
    }
    void Succeed(const Char* s)
    {
      Assign(S_OK, s);
    }

    // --------------------------------- Query
    operator bool() const
    {
      return Succeeded();
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



