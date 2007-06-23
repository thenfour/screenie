#ifndef SCREENIE_STDAFX_HPP
#define SCREENIE_STDAFX_HPP

#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_DEPRECATE

#define WINVER				0x0500
#define _WIN32_WINNT		0x0500
#define _WIN32_IE			0x0500
#define _RICHEDIT_VER		0x0100

#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>

#include <atlbase.h>
#include "..\wtl\atlapp.h"

extern CAppModule _Module;

#include <atlwin.h>

#include "..\wtl\atlframe.h"
#include "..\wtl\atlctrls.h"
#include "..\wtl\atlctrlx.h"
#include "..\wtl\atldlgs.h"
#include "..\wtl\atlscrl.h"
#include "..\wtl\atlmisc.h"
#include "..\wtl\atluser.h"

#include <gdiplus.h>

#include "tstdlib/tstring.hpp"
#include "tstdlib/tsstream.hpp"

#include "libcc/stringutil.h"
#include "libcc/result.h"
#include "libcc/log.h"

#include "../sqlite/sqlite3x.hpp"

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