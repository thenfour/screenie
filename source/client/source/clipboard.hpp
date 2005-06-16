#ifndef SCREENIE_CLIPBOARD_HPP
#define SCREENIE_CLIPBOARD_HPP

#include "exception.hpp"

struct Clipboard
{
	Clipboard(HWND owner)
	{
		if (!::OpenClipboard(owner))
			throw Win32Exception(GetLastError());
	}

	~Clipboard()
	{
		::CloseClipboard();
	}

	void SetText(const tstd::tstring& text)
	{
    EmptyClipboard();
    size_t nSize;
    HANDLE hMem;
    PVOID hMemLoc;

    // unicode first
    nSize = (text.length() + 1) * sizeof(WCHAR);
    hMem = GlobalAlloc(GMEM_MOVEABLE, nSize);
		if (hMem == NULL) throw Win32Exception(GetLastError());
    hMemLoc = GlobalLock(hMem);
    LibCC::StringCopyN((WCHAR*)hMemLoc, text.c_str(), text.length() + 1);
    GlobalUnlock(hMem);
    if(NULL == SetClipboardData(CF_UNICODETEXT, hMem)) throw Win32Exception(GetLastError());

    // ascii
    nSize = (text.length() + 1) * sizeof(char);
    hMem = GlobalAlloc(GMEM_MOVEABLE, nSize);
		if (hMem == NULL) throw Win32Exception(GetLastError());
    hMemLoc = GlobalLock(hMem);
    LibCC::StringCopyN((char*)hMemLoc, text.c_str(), text.length() + 1);
    GlobalUnlock(hMem);
    if(NULL == SetClipboardData(CF_TEXT, hMem)) throw Win32Exception(GetLastError());
	}

	void SetBitmap(HBITMAP bitmap)
	{
		if (bitmap == NULL)
			throw Win32Exception(GetLastError());

		if (::SetClipboardData(CF_BITMAP, (HANDLE)bitmap) == NULL)
			throw Win32Exception(GetLastError());
	}

	bool GetText(tstd::tstring& text)
	{
		return true;
	}

	bool GetBitmap(HBITMAP& bitmap)
	{
		return true;
	}
};

#endif