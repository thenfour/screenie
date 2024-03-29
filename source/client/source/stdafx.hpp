// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark

#ifndef SCREENIE_STDAFX_HPP
#define SCREENIE_STDAFX_HPP

#define NOMINMAX

#define UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_DEPRECATE

#define WINVER				0x0600
#define _WIN32_WINNT		0x0600

#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <algorithm>

using std::max;
using std::min;

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

// These classes are used from the 'util' namespace so that if the backing
// implementation changes, the codebase can always use this interface.
#include <memory>

namespace Gdiplus
{
	typedef std::tr1::shared_ptr<Gdiplus::Bitmap> BitmapPtr;
}

#endif