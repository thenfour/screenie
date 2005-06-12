//
// internet.hpp - WinInet wrappers and utility functions
// Copyright (c) 2003 Carl Corcoran
// Copyright (c) 2005 Roger Clark
//

#ifndef SCREENIE_INTERNET_HPP
#define SCREENIE_INTERNET_HPP

#include <wininet.h>

// Exception
#include "exception.hpp"

// GetWinInetErrorString()
#include "utility.hpp"

class WinInetException : public Exception
{
public:
	WinInetException()
	{
		errorString = GetWinInetErrorString();
	}

	virtual ~WinInetException() { }

	tstd::tstring What() const { return errorString; }
private:
	tstd::tstring errorString;
};

struct WinInetHandle
{
	WinInetHandle(HINTERNET handleIn) : handle(handleIn) { }
	WinInetHandle(WinInetHandle& copy) { operator=(copy); }

	~WinInetHandle()
	{
		if (handle != NULL)
			::InternetCloseHandle(handle);
	}

	WinInetHandle& operator=(WinInetHandle& rightHand)
	{
		handle = rightHand.handle;
		rightHand.handle = NULL;

		return (*this);
	}

	WinInetHandle& operator=(HINTERNET rightHand)
	{
		handle = rightHand;
		return (*this);
	}

	HINTERNET handle;
};

inline tstd::tstring InternetGetLastResponseInfoX(DWORD* pdwError = 0)
{
  DWORD size = 0;
  TCHAR temp;
  DWORD dwError;
  InternetGetLastResponseInfo(&dwError, &temp, &size);
  if(size < 100000)// sanity check
  {
    LibCC::Blob<TCHAR> buf;
    size ++;
    InternetGetLastResponseInfo(&dwError, buf.GetBuffer(size + 2), &size);
    if(pdwError)
    {
      *pdwError = dwError;
    }
    return buf.GetBuffer();
  }
  return _T("internal error");
}


#endif