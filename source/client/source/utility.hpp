//
// utility.hpp - miscellaneous utility functions
// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
//

#ifndef SCREENIE_UTILITY_HPP
#define SCREENIE_UTILITY_HPP

// for std::vector
#include <vector>

// for tstd::tstring
#include "libcc/stringutil.hpp"

bool MakeDestinationFilename(tstd::tstring& filename, const SYSTEMTIME& systemTime, 
	const tstd::tstring& mimeType, const tstd::tstring& formatString);

tstd::tstring GetUniqueTemporaryFilename(const tstd::tstring& extension = _T("tmp"));
tstd::tstring GetWinInetErrorString();

void AutoSetComboBoxHeight(CComboBox& c);

BOOL EnableChildWindows(HWND parent, BOOL enable);

bool GetSpecialFolderPath(tstd::tstring& path, int folder);
tstd::tstring GetUniqueFilename(tstd::tstring path);

tstd::tstring GetWindowString(HWND hwnd);
tstd::tstring GetComboSelectionString(HWND hWndCombo);

tstd::tstring tstring_tolower(const tstd::tstring& input);
tstd::tstring tstring_toupper(const tstd::tstring& input);

inline bool IsValidHandle(HANDLE h)
{
	return (h != 0) && (h != INVALID_HANDLE_VALUE);
}

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


inline std::wstring GetNumberFormatX(std::wstring input)
{
	LibCC::Blob<wchar_t> retBlob;
	GetNumberFormat(LOCALE_USER_DEFAULT, 0, input.c_str(), 0, retBlob.GetBuffer(100), 99);
	return retBlob.GetBuffer();
}
//
//inline std::wstring FormatSize(DWORD dwFileSize)
//{
//  static const DWORD dwKB = 1024;
//  static const DWORD dwMB = 1024 * dwKB;
//  static const DWORD dwGB = 1024 * dwMB;
//
//  if (dwFileSize < dwKB)
//  {
//		return LibCC::Format("% b")(GetNumberFormatX(LibCC::Format().f<2,2>((float)dwFileSize).Str())).Str();
//  }
//  if (dwFileSize < dwMB)
//  {
//		return LibCC::Format("% kb")(GetNumberFormatX(LibCC::Format().f<2,2>(((float)dwFileSize) / dwKB).Str())).Str();
//  }
//  if (dwFileSize < dwGB)
//  {
//		return LibCC::Format("% mb")(GetNumberFormatX(LibCC::Format().f<2,2>(((float)dwFileSize) / dwMB).Str())).Str();
//  }
//	return LibCC::Format("% gb")(GetNumberFormatX(LibCC::Format().f<2,2>(((float)dwFileSize) / dwGB).Str())).Str();
//}

/*
	1 bytes
	1023 bytes
	1.0 kb
	9.9 kb
	10 kb
	1023 kb
	1.0 mb
	9.9 mb
*/
inline std::wstring BytesToString(size_t bytes)
{
	if(bytes < 1024)
		return LibCC::Format(L"% bytes").ui(bytes).Str();
	PCWSTR suffixes[] = { L"kb", L"mb", L"gb" };
	int lastSuffix = LibCC::SizeofStaticArray(suffixes) - 1;
	double size = (double)bytes / 1024;
	int suffix = 0;
	while(true)
	{
		if(size < 10)
		{
			return LibCC::Format(L"% %").d<1>(size).s(suffixes[suffix]).Str();
		}
		if(size < 1024 || suffix == lastSuffix)
		{
			return LibCC::Format(L"% %").d<0>(size).s(suffixes[suffix]).Str();
		}

		size /= 1024;
		suffix ++;
	}
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
		std::wstring W = LibCC::ToUTF16(x);
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
    tstd::tstring ret;
		LibCC::StringConvert(p, ret);
    CoTaskMemFree(p);
    return ret;
  }
};

// used to ensure that different filenames in the same destination will be the same
struct ScreenshotNamingData
{
	ScreenshotNamingData();

	SYSTEMTIME& UsableTime(bool useLocalTime) { return useLocalTime ? localTime : utcTime; }
	SYSTEMTIME utcTime;
	SYSTEMTIME localTime;
	std::wstring windowTitle;
	std::vector<Guid> guids;
	std::vector<std::wstring> userStrings;
};

tstd::tstring FormatFilename(ScreenshotNamingData& namingData, bool useLocalTime, const tstd::tstring& inputFormat, bool preview = false);

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
//typedef bool (*UploadFTPFileProgressProc_T)(DWORD completed, DWORD total, void* pUser);
//LibCC::Result UploadFTPFile(struct ScreenshotDestination& dest, const tstd::tstring& localFile, const tstd::tstring& remoteFile, DWORD bufferSize, UploadFTPFileProgressProc_T pProc, void* pUser, OUT DWORD* size);

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



class MemStream
{
public:
	IStream* m_pStream;

	void GetBlob(LibCC::Blob<BYTE>& out)
	{
		out.Alloc(GetLength());

		LARGE_INTEGER li;
		ULARGE_INTEGER uli;
		li.QuadPart = 0;
		m_pStream->Seek(li, STREAM_SEEK_SET, &uli);
		ULONG ulr;
		m_pStream->Read(out.GetBuffer(), out.Size(), &ulr);
	}

	size_t GetLength()
	{
		STATSTG stg = {0};
		m_pStream->Stat(&stg, STATFLAG_NONAME);
		return (size_t)stg.cbSize.QuadPart;
	}

	MemStream()
	{
		CreateStreamOnHGlobal(0, TRUE, &m_pStream);
		LARGE_INTEGER li;
		ULARGE_INTEGER uli;
		li.QuadPart = 0;
		m_pStream->Seek(li, STREAM_SEEK_SET, &uli);
	}

	MemStream(BYTE* data, size_t size)
	{
		CreateStreamOnHGlobal(0, TRUE, &m_pStream);
		LARGE_INTEGER li;
		ULARGE_INTEGER uli;
		li.QuadPart = 0;
		m_pStream->Seek(li, STREAM_SEEK_SET, &uli);
		m_pStream->Write(data, (ULONG)size, 0);
		m_pStream->Seek(li, STREAM_SEEK_SET, &uli);
	}

	MemStream(HINSTANCE hInstance, LPCTSTR res, PCTSTR type)
	{
		CreateStreamOnHGlobal(0, TRUE, &m_pStream);
		LARGE_INTEGER li;
		ULARGE_INTEGER uli;
		li.QuadPart = 0;
		m_pStream->Seek(li, STREAM_SEEK_SET, &uli);

		HRSRC hrsrc=FindResource(hInstance, res, type);
		HGLOBAL hg1 = LoadResource(hInstance, hrsrc);
		DWORD sz = SizeofResource(hInstance, hrsrc);
		void* ptr1 = LockResource(hg1);

		m_pStream->Write(ptr1, sz, 0);
		m_pStream->Seek(li, STREAM_SEEK_SET, &uli);
	}
	~MemStream()
	{
		m_pStream->Release();
	}
};

inline std::wstring LoadTextFileResource(HINSTANCE hInstance, LPCTSTR szResName, LPCTSTR szResType)
{
    HRSRC hrsrc=FindResource(hInstance, szResName, szResType);
    if(!hrsrc) return L"";
    HGLOBAL hg1 = LoadResource(hInstance, hrsrc);
    DWORD sz = SizeofResource(hInstance, hrsrc);
    void* ptr1 = LockResource(hg1);

		// assume the encoding is ASCII.
		std::string a((const char*)ptr1, sz);
		return LibCC::ToUTF16(a);
} 


CFont& UtilGetShellFont();


// converts strins to tstd::tstring
inline tstd::tstring ToTstring(const std::basic_string<wchar_t>& in)
{
#ifdef _UNICODE
	return in;
#else
	return LibCC::ToANSI(in);
#endif
}

inline tstd::tstring ToTstring(const std::basic_string<char>& in)
{
#ifdef _UNICODE
	return LibCC::ToUTF16(in);
#else
	return in;
#endif
}



#endif



