// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#undef UNICODE
#define UNICODE
#undef _UNICODE
#define _UNICODE

#define _CRT_SECURE_NO_DEPRECATE

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
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

#include "../client/source/libcc/stringutil.hpp"
#include "../client/source/libcc/result.hpp"
#include "../client/source/libcc/log.hpp"
#include "../client/source/libcc/timer.hpp"


