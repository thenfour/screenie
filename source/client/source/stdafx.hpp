#ifndef SCREENIE_STDAFX_HPP
#define SCREENIE_STDAFX_HPP

#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_DEPRECATE

#define WINVER				0x0501
#define _WIN32_WINNT		0x0501
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
#include "..\wtl\atlsplit.h"

#include <gdiplus.h>

#include "tstdlib/tstring.hpp"
#include "tstdlib/tsstream.hpp"

#include "libcc/stringutil.hpp"
#include "libcc/result.hpp"
#include "libcc/log.hpp"

#include "../sqlite/sqlite3x.hpp"

// this header contains Graham Batty's implementation of the C++
// Technical Report 1 smart pointers (shared_ptr and weak_ptr).
//
// <carl> i don't get it. build fails with this commented. uncommenting. is there a built-in shared_ptr in vc2008 i don't know about? if so, how do we enable it?
//
#include "tr1/memory"// -- now use VC2008's =[

// These classes are used from the 'util' namespace so that if the backing
// implementation changes, the codebase can always use this interface.

namespace util
{
	using std::tr1::shared_ptr;
	using std::tr1::weak_ptr;
};

namespace Gdiplus
{
	typedef std::tr1::shared_ptr<Gdiplus::Bitmap> BitmapPtr;
}

#endif