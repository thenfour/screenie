//
// utility.hpp - miscellaneous utility functions
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#ifndef SCREENIE_UTILITY_HPP
#define SCREENIE_UTILITY_HPP

// for std::vector
#include <vector>

// for tstd::tstring
#include "tstdlib/tstring.hpp"

bool MakeDestinationFilename(tstd::tstring& filename,
	const tstd::tstring& mimeType, const tstd::tstring& formatString);

tstd::tstring GetUniqueTemporaryFilename();
tstd::tstring GetWinInetErrorString();

tstd::tstring GetLastErrorString(DWORD lastError);

bool GetStringResource(HINSTANCE instance, UINT id, tstd::tstring& stringOut);

BOOL EnableChildWindows(HWND parent, BOOL enable);

bool GetSpecialFolderPath(tstd::tstring& path, int folder);
tstd::tstring GetUniqueFilename(tstd::tstring path);

tstd::tstring GetWindowString(HWND hwnd);
tstd::tstring GetComboSelectionString(HWND hWndCombo);

tstd::tstring FormatFilename(const SYSTEMTIME& systemTime, const tstd::tstring& inputFormat);

tstd::tstring tstring_tolower(const tstd::tstring& input);
tstd::tstring tstring_toupper(const tstd::tstring& input);

class Guid
{
public:
  GUID val;

  Guid()
  {
    memset(&val, 0, sizeof(val));
  }
  Guid(const Guid& x) { Assign(x); }
  Guid(const GUID& x) { Assign(x); }

  Guid& operator =(const Guid& x) { return Assign(x); }
  Guid& operator =(const GUID& x) { Assign(x); }
  Guid& operator =(const tstd::tstring& x) { Assign(x); }

  bool operator ==(const Guid& x) const  { return Equals(x); }
  bool operator !=(const Guid& x) const  { return !Equals(x); }
  Guid& Assign(const tstd::tstring& x)
  {
    std::wstring W = tstd::convert<wchar_t>(x);
    LibCC::Blob<OLECHAR> crap;
    crap.Alloc(W.size()+1);
    wcscpy(crap.GetBuffer(), W.c_str());
    CLSIDFromString(crap.GetBuffer(), &val);
    return *this;
  }
  Guid& Assign(const Guid& x)
  {
    memcpy(&val, &x.val, sizeof(GUID));
    return *this;
  }
  Guid& Assign(const GUID& x)
  {
    memcpy(&val, &x, sizeof(GUID));
    return *this;
  }
  bool Equals(const Guid& x) const
  {
    return (memcmp(&x.val, &val, sizeof(GUID)) == 0);
  }
  void CreateNew()
  {
    CoCreateGuid(&val);
  }
  tstd::tstring ToString() const
  {
    LPOLESTR p;
    StringFromCLSID(val, &p);
    tstd::tstring ret = tstd::convert<tstd::tchar_t>(p);
    CoTaskMemFree(p);
    return ret;
  }
};

// RAII critsec class
class CriticalSection
{
public:
  CriticalSection() { InitializeCriticalSection(&m_cs); }
  ~CriticalSection() { DeleteCriticalSection(&m_cs); }
  bool Enter() { EnterCriticalSection(&m_cs); return true; }
  bool Leave() { LeaveCriticalSection(&m_cs); return true; }

  class ScopeLock
  {
  public:
    ScopeLock(CriticalSection& x) : m_cs(x) { m_cs.Enter(); }
    ~ScopeLock() { m_cs.Leave(); }
  private:
    CriticalSection& m_cs;
  };
private:
  CRITICAL_SECTION m_cs;
};


#endif