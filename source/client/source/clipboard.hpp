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
		HGLOBAL clipboardData = ::GlobalAlloc(GMEM_MOVEABLE,
			(text.length() + 1) * sizeof(TCHAR));

		if (clipboardData == NULL)
			throw Win32Exception(GetLastError());

		TCHAR* clipboardText = reinterpret_cast<TCHAR*>(::GlobalLock(clipboardData));
		_tcscpy(clipboardText, text.c_str());
		::GlobalUnlock(clipboardData);

		if (::SetClipboardData(CF_TEXT, reinterpret_cast<HANDLE>(clipboardData)) == NULL)
			throw Win32Exception(GetLastError());
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