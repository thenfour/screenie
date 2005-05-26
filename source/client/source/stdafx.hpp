#ifndef SCREENIE_STDAFX_HPP
#define SCREENIE_STDAFX_HPP

#define WINVER				0x0500
#define _WIN32_WINNT		0x0500
#define _WIN32_IE			0x0500
#define _RICHEDIT_VER		0x0100

#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlscrl.h>
#include <atlmisc.h>
#include <atluser.h>

#include <gdiplus.h>

#include "tstdlib/tstring.hpp"
#include "tstdlib/tsstream.hpp"

#include <libcc/ccstr.h>

// this header contains Graham Batty's implementation of the C++
// Technical Report 1 smart pointers (shared_ptr and weak_ptr).
#include "tr1/memory"

// These classes are used from the 'util' namespace so that if the backing
// implementation changes, the codebase can always use this interface.

namespace util
{
	using std::tr1::shared_ptr;
	using std::tr1::weak_ptr;
};

#endif