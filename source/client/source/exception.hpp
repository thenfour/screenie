// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark

#ifndef SCREENIE_EXCEPTION_HPP
#define SCREENIE_EXCEPTION_HPP

// GetLastErrorString()
#include "utility.hpp"

struct Exception
{
	Exception() { }
	virtual ~Exception() { }

	virtual tstd::tstring What() const = 0;
};

class Win32Exception : public Exception
{
public:
	Win32Exception(DWORD lastErrorIn) : lastError(lastErrorIn)
	{
		errorString = LibCC::Format().gle(lastErrorIn).Str();
	}

	virtual ~Win32Exception() { }

	DWORD LastError() const { return lastError; }
	tstd::tstring What() const { return errorString; }
private:
	DWORD lastError;
	tstd::tstring errorString;
};

#endif