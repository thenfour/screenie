//
// screenie.cpp - entry point for the app, mostly generated by the wtl wizard
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#include "stdafx.hpp"
#include "resource.h"

#include "image.hpp"
#include "animbitmap.h"
#include "utility.hpp"

#include "aboutdlg.hpp"
#include "MainWindow.hpp"
#include "viewport.h"
#include "DestinationDlg.hpp"
#include "ScreenshotArchive.hpp"

extern void TestMain();

CMainWindow* g_mainWindow;
CAppModule _Module;

HHOOK g_keyboardHook;

namespace LibCC
{
	Log* g_pLog = 0;
}

LRESULT CALLBACK PrintScreenProc(int code, WPARAM wParam, LPARAM lParam)
{
	KBDLLHOOKSTRUCT* ks = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
	LRESULT result = ::CallNextHookEx(g_keyboardHook, code, wParam, lParam);

	if (code == HC_ACTION)
	{
		if ((ks->vkCode == VK_SNAPSHOT) &&
			((wParam == WM_KEYDOWN) || (wParam == WM_SYSKEYDOWN)))
		{
			POINT cursorPos = { 0 };
			::GetCursorPos(&cursorPos);

			BOOL altDown = (ks->flags & LLKHF_ALTDOWN);

			::PostMessage(*g_mainWindow, CMainWindow::WM_TAKESCREENSHOT,
				MAKELONG(cursorPos.x, cursorPos.y), altDown);
		}
	}

	return result;
}

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	bool logEnabled = false;
#ifdef _DEBUG
	logEnabled = true;
#endif
	LibCC::g_pLog = new LibCC::Log(GetPathRelativeToApp(_T("screenie.log")), _Module.GetResourceInstance(), logEnabled, logEnabled, logEnabled);

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	int retval = 0;

  ScreenshotOptions options;
	options.LoadSettings();

	ScreenshotArchive archive(options);

  g_mainWindow = new CMainWindow(options, archive);

	RECT display = { 0 };
	if (g_mainWindow->Create(NULL, display, TEXT("ScreenieWnd"), WS_POPUP))
	{
		g_keyboardHook = ::SetWindowsHookEx(WH_KEYBOARD_LL, PrintScreenProc,
			_Module.GetModuleInstance(), 0);

		if (g_keyboardHook != NULL)
		{
			retval = theLoop.Run();
		}
	}

	options.SaveSettings();

  delete g_mainWindow;

	_Module.RemoveMessageLoop();

	delete LibCC::g_pLog;
	return retval;
}

int WINAPI _tWinMain(HINSTANCE instance, HINSTANCE, LPTSTR cmdLine, int showCmd)
{
	//TestMain();
	//return 0;

	// try to create a global mutex object. if it already exists, it means there's
	// another instance of this program already running, and we shouldn't run another.

	HANDLE instanceMutex = ::CreateMutex(NULL, TRUE, TEXT("ScreenieInstMutex"));

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
    MessageBox(0, _T("Screenie is already running."), _T("Screenie"), MB_OK | MB_ICONINFORMATION);

		HWND screenieWindow = ::FindWindow(TEXT("ScreenieMainWnd"), TEXT("ScreenieWnd"));

		if (::IsWindow(screenieWindow))
		{
			::SendMessage(screenieWindow, CMainWindow::WM_SHOWSCREENIE, 0, 0);

			// if there's another instance, it won't hurt us to return without closing
			// the mutex handle. the previous instance would be responsible for releasing it.

			return 0;
		}
	}

	HRESULT hRes = ::CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	// add flags to support other controls
	AtlInitCommonControls(ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES);

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;

	int nRet = 0;

	Gdiplus::Status status = Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	if (status == Gdiplus::Ok)
	{
		if (SUCCEEDED(_Module.Init(NULL, instance)))
		{
			nRet = Run(cmdLine, showCmd);

			_Module.Term();
		}

		Gdiplus::GdiplusShutdown(gdiplusToken);
	}
	else
	{
		::MessageBox(NULL, LibCC::Format(TEXT("Can't initialize GDI+: %")).s(GetGdiplusStatusString(status)).CStr(),
			TEXT("Fatal Error"), MB_OK);
	}

	::CoUninitialize();

	if (instanceMutex != NULL)
		::CloseHandle(instanceMutex);

	return nRet;
}
