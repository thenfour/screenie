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
#include "libcc/stringutil.h"

bool MakeDestinationFilename(tstd::tstring& filename, const SYSTEMTIME& systemTime, 
	const tstd::tstring& mimeType, const tstd::tstring& formatString);

tstd::tstring GetUniqueTemporaryFilename();
tstd::tstring GetWinInetErrorString();

void AutoSetComboBoxHeight(CComboBox& c);

BOOL EnableChildWindows(HWND parent, BOOL enable);

bool GetSpecialFolderPath(tstd::tstring& path, int folder);
tstd::tstring GetUniqueFilename(tstd::tstring path);

tstd::tstring GetWindowString(HWND hwnd);
tstd::tstring GetComboSelectionString(HWND hWndCombo);

tstd::tstring FormatFilename(const SYSTEMTIME& systemTime, const tstd::tstring& inputFormat,
							 const tstd::tstring& windowTitle, bool preview = false);

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
		std::wstring W = LibCC::ToUnicode(x);
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
		LibCC::ConvertString(p, ret);
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



class BlobStream : public IStream
{
public:
	size_t m_cursor;
	LibCC::Blob<BYTE> m_blob;

	BlobStream() :
		m_cursor(0)
	{
	}

	void* GetBuffer() const 
	{
		return (void*)m_blob.GetBuffer();
	}
	int GetLength() const
	{
		return m_blob.Size();
	}

	// IUnknown
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
	{
		if(riid == IID_IUnknown)
		{
			*ppvObject = (IUnknown*)this; 
			return S_OK;
		}
		if(riid == IID_IStream)
		{
			*ppvObject = (IStream*)this; 
			return S_OK;
		}
		return E_NOINTERFACE;
	}
  ULONG STDMETHODCALLTYPE AddRef( void)
	{
		return 1;
	}
  ULONG STDMETHODCALLTYPE Release( void)
	{
		return 1;
	}

	// IStream
  HRESULT STDMETHODCALLTYPE Read(void *pv, ULONG cb, ULONG *pcbRead)
	{
		if(m_cursor >= m_blob.Size())
		{
			return STG_E_INVALIDPOINTER;
		}
		size_t sizeLeft = m_blob.Size() - m_cursor;
		size_t toRead = min(cb, sizeLeft);
		memcpy(pv, m_blob.GetBuffer() + m_cursor, toRead);
		m_cursor += toRead;
		if(pcbRead)
			*pcbRead = toRead;
		return toRead == cb ? S_OK : S_FALSE;
	}
	  
	HRESULT STDMETHODCALLTYPE Write(const void *pv, ULONG cb, ULONG *pcbWritten)
	{
		if(m_cursor > m_blob.Size())
		{
			return STG_E_INVALIDPOINTER;
		}
		size_t cursorAfter = m_cursor + cb;
		if(cursorAfter > m_blob.Size())
		{
			m_blob.Alloc(cursorAfter);
		}
		memcpy(m_blob.GetBuffer() + m_cursor, pv, cb);
		*pcbWritten = cb;
		m_cursor = cursorAfter;
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE Seek( LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
	{
		if(dlibMove.HighPart > 0)
		{
			return STG_E_INVALIDPOINTER;
		}

		switch(dwOrigin)
		{
		case STREAM_SEEK_CUR:
			m_cursor += (int)dlibMove.LowPart;
			break;
		case STREAM_SEEK_END:
			m_cursor = m_blob.Size() + (int)dlibMove.LowPart;
			break;
		case STREAM_SEEK_SET:
			m_cursor = dlibMove.LowPart;
			break;
		default:
			return STG_E_INVALIDFUNCTION;
		}
		if(m_cursor > m_blob.Size())
		{
			return STG_E_INVALIDPOINTER;
		}
		
		if(plibNewPosition)
			plibNewPosition->QuadPart = 0;

		return S_OK;
	}
  
  HRESULT STDMETHODCALLTYPE SetSize( ULARGE_INTEGER libNewSize)
	{
		if(libNewSize.HighPart > 0) return STG_E_MEDIUMFULL;
		m_blob.Alloc(libNewSize.LowPart);
		return S_OK;
	}
  
  HRESULT STDMETHODCALLTYPE CopyTo(  IStream *pstm, ULARGE_INTEGER cb,ULARGE_INTEGER *pcbRead,ULARGE_INTEGER *pcbWritten)
	{
		if(m_cursor > m_blob.Size())
		{
			return STG_E_INVALIDPOINTER;
		}
		if(cb.HighPart > 0) return STG_E_MEDIUMFULL;

		ULONG bw = 0;
		HRESULT hr = pstm->Write(m_blob.GetBuffer() + m_cursor, cb.LowPart, &bw);
		if(pcbWritten)
		{
			pcbWritten->HighPart = 0;
			pcbWritten->LowPart = bw;
		}
		return hr;
	}
  
  HRESULT STDMETHODCALLTYPE Commit( DWORD grfCommitFlags)
	{
		return S_OK;
	}
  
  HRESULT STDMETHODCALLTYPE Revert( void)
	{
		return E_NOTIMPL;
	}
  
  HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
	{
		return E_NOTIMPL;
	}
  
  HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
	{
		return E_NOTIMPL;
	}
  
  HRESULT STDMETHODCALLTYPE Stat(  STATSTG *pstatstg,DWORD grfStatFlag)
	{
		if(grfStatFlag & STATFLAG_NONAME)
		{
			// uh i should really set more stuff here, but i don't need to at the moment.
			pstatstg->cbSize.QuadPart = m_blob.Size();
			return S_OK;
		}
		return E_NOTIMPL;
	}
  
  HRESULT STDMETHODCALLTYPE Clone( IStream **ppstm)
	{
		return E_NOTIMPL;
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
		return LibCC::ToUnicode(a);
} 


CFont& UtilGetShellFont();


#endif



