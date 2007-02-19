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

bool MakeDestinationFilename(tstd::tstring& filename, const SYSTEMTIME& systemTime, 
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

tstd::tstring FormatFilename(const SYSTEMTIME& systemTime, const tstd::tstring& inputFormat, bool preview = false);

tstd::tstring tstring_tolower(const tstd::tstring& input);
tstd::tstring tstring_toupper(const tstd::tstring& input);

inline tstd::tstring GetPathRelativeToApp(PCTSTR extra)
{
  TCHAR sz[MAX_PATH];
  TCHAR sz2[MAX_PATH];
  GetModuleFileName(0, sz, MAX_PATH);
  _tcscpy(PathFindFileName(sz), extra);
  PathCanonicalize(sz2, sz);
  return sz2;
}

inline tstd::tstring GetModuleFileNameX()
{
  TCHAR sz[MAX_PATH];
  GetModuleFileName(0, sz, MAX_PATH);
  return sz;
}

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

struct Win32Handle
{
	Win32Handle(HANDLE h) : val(h) { }
	Win32Handle(Win32Handle& copy) { operator=(copy); }

	~Win32Handle()
	{
		if(val != NULL)
      CloseHandle(val);
	}

	Win32Handle& operator=(Win32Handle& rhs)
	{
    val = rhs.val;
    rhs.val = NULL;
		return (*this);
	}

	Win32Handle& operator=(HANDLE rhs)
	{
		val = rhs;
		return (*this);
	}

	HANDLE val;
};

// return false to cancel uploading
// return true to continue
typedef bool (*UploadFTPFileProgressProc_T)(DWORD completed, DWORD total, void* pUser);
LibCC::Result UploadFTPFile(struct ScreenshotDestination& dest, const tstd::tstring& localFile, const tstd::tstring& remoteFile, DWORD bufferSize, UploadFTPFileProgressProc_T pProc, void* pUser);

#ifdef UNICODE
typedef tagMENUITEMINFOW tagMENUITEMINFO; 
#else
typedef tagMENUITEMINFOA tagMENUITEMINFO;
#endif

struct MenuItemInfo :
  public tagMENUITEMINFO
{
  // initialize with a separator
  MenuItemInfo()
  {
  }

  MenuItemInfo(const MenuItemInfo& rhs)
  {
    MENUITEMINFO* plhs = this;
    const MENUITEMINFO* prhs = &rhs;
    memcpy(plhs, prhs, sizeof(MENUITEMINFO));
    if(rhs.dwTypeData == rhs.m_string.GetBuffer())
    {
      StringCopy(rhs.m_string.GetBuffer());
      dwTypeData = m_string.GetBuffer();
    }
  }

  MenuItemInfo(const MENUITEMINFO& rhs)
  {
    MENUITEMINFO* plhs = this;
    memcpy(plhs, &rhs, sizeof(MENUITEMINFO));
  }

  operator MENUITEMINFO*()
  {
    return this;
  }

  MenuItemInfo& operator =(const MENUITEMINFO& rhs)
  {
    MENUITEMINFO* plhs = this;
    memcpy(plhs, &rhs, sizeof(MENUITEMINFO));
  }

  // stores a string in the instance
  TCHAR* StringCopy(const tstd::tstring& text)
  {
    m_string.Alloc(text.size() + 1);
    _tcscpy(m_string.GetBuffer(), text.c_str());
    return m_string.GetBuffer();
  }

  // creates a new separator item
  static MenuItemInfo CreateSeparator()
  {
    MenuItemInfo ret;
    ret.cbSize = sizeof(MENUITEMINFO);
    ret.fMask = MIIM_TYPE | MIIM_ID;
    ret.fType = MFT_SEPARATOR; 
    return ret;
  }

  // creates a new normal text item
  static MenuItemInfo CreateText(const tstd::tstring& text, UINT uCmd)
  {
    MenuItemInfo ret;
    ret.cbSize = sizeof(MENUITEMINFO);
    ret.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID;
    ret.fType = MFT_STRING;
    ret.wID = uCmd;
    ret.dwTypeData = ret.StringCopy(text);
    return ret;
  }

protected:
  LibCC::Blob<TCHAR> m_string;// why blob?  because freaking menuitem structs take non-const strings, so this is the easiest way to be good-bear
};

#endif



