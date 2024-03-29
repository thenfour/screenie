//
// screenie.cpp - entry point for the app, mostly generated by the wtl wizard
// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
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

#include "libcc/registry.hpp"

#include "version.h"
//#include "..\libcrashman\libcrashman.h"

// UNCOMMENT THIS TO RUN TESTMAIN() INSTEAD OF THE NORMAL WINMAIN
//#define TESTMODE


extern void TestMain();

CMainWindow* g_mainWindow;
CAppModule _Module;

HHOOK g_keyboardHook;

namespace LibCC
{
	Log* g_pLog = 0;
}

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	bool logFileEnabled = false;
	bool logWindowEnabled = false;
	bool logOutputEnabled = false;
#ifdef _DEBUG
	logFileEnabled = true;
	logWindowEnabled = true;
	logOutputEnabled = true;
#endif
	LibCC::g_pLog = new LibCC::Log(GetPathRelativeToApp(_T("screenie.log")), _Module.GetResourceInstance(), logOutputEnabled, logWindowEnabled, logFileEnabled);

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	int retval = 0;

	ScreenshotOptions options;
	if (!options.LoadSettings())
	{
		// add an imageshack destination by default
		ScreenshotDestination dest;

		dest.enabled = true;
		dest.general.type = ScreenshotDestination::TYPE_IMAGESHACK;
		dest.general.imageFormat = _T("image/png");
		dest.general.name = _T("ImageShack Upload");
		dest.imageshack.copyURL = true;

		options.AddDestination(dest);
	}

	ScreenshotArchive archive(options);

  g_mainWindow = new CMainWindow(options, archive);

	RECT display = { 0 };
	if (g_mainWindow->Create(NULL, display, TEXT("ScreenieWnd"), WS_POPUP))
	{
		retval = theLoop.Run();
	}

	options.SaveSettings();

  delete g_mainWindow;

	_Module.RemoveMessageLoop();

	delete LibCC::g_pLog;
	return retval;
}

//void PopulateSystemInfo(CrashMan::StringMap& strings)
//{
//	// windows version
//	{
//		OSVERSIONINFO ex;
//		ex.dwOSVersionInfoSize = sizeof(ex);
//		::GetVersionEx(&ex);
//
//		strings[L"osversion"] = LibCC::FormatW(L"Windows NT %.%.% %")
//			.i(ex.dwMajorVersion)
//			.i(ex.dwMinorVersion)
//			.i(ex.dwBuildNumber)
//			.s(ex.szCSDVersion).Str();
//	}
//
//	// internet explorer version
//	{
//		LibCC::RegistryKey key(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Internet Explorer");
//		std::wstring iever;
//		key.GetValue(L"Version", iever);
//		strings[L"ieversion"] = iever;
//	}
//
//	// number of processors
//	{
//		SYSTEM_INFO si = { 0 };
//		::GetNativeSystemInfo(&si);
//
//		strings[L"cpus"] = LibCC::FormatW(L"%").i(si.dwNumberOfProcessors).Str();
//	}
//
//	// processor
//	{
//		LibCC::RegistryKey key(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0");
//		std::wstring cpuname;
//		key.GetValue(L"ProcessorNameString", cpuname);
//		strings[L"cpuname"] = cpuname;
//	}
//
//	// memory
//	{
//		MEMORYSTATUSEX statex = { 0 };
//		statex.dwLength = sizeof(statex);
//		GlobalMemoryStatusEx(&statex);
//
//		strings[L"physmem"] = LibCC::FormatW(L"% MB").ul(statex.ullTotalPhys / 1024 / 1024).Str();
//	}
//}
//
//bool ScreenieCrashHandler(const wchar_t* dumpfile, EXCEPTION_POINTERS* pExInfo, void* pContext)
//{
//	std::wstring exepath = LibCC::PathRemoveFilename(GetModuleFileNameX());
//	LibCC::PathAppendX(exepath, L"crashman.exe");
//
//	std::wstring inipath = LibCC::PathRemoveFilename(GetModuleFileNameX());
//	LibCC::PathAppendX(inipath, L"crashman.ini");
//
//	if (LibCC::PathFileExistsX(dumpfile) &&
//		LibCC::PathFileExistsX(exepath.c_str()) &&
//		LibCC::PathFileExistsX(inipath.c_str()))
//	{
//		CrashMan::StringMap params;
//		PopulateSystemInfo(params);
//		params[L"svnrevision"] = LibCC::FormatW(L"%").i(TSVN_VERMINOR).Str();
//
//		CrashMan::StringMap strings;
//		strings[L"url"] = L"http://screenie.net/debug/dump.php";
//
//		if (CrashMan::Execute(exepath.c_str(), dumpfile, params, strings,
//			GetModuleFileNameX().c_str(), L"/restart", inipath.c_str()))
//		{
//			return true;
//		}
//	}
//
//	return false;
//}
//
//
//struct AutoCrashMan
//{
//	AutoCrashMan(CrashMan::CrashHandler handler)
//	{
//		CrashMan::Initialize(handler);
//	}
//
//	~AutoCrashMan()
//	{
//		CrashMan::Uninitialize();
//	}
//};


#ifndef DPI_ENUMS_DECLARED
typedef enum PROCESS_DPI_AWARENESS
{
	PROCESS_DPI_UNAWARE = 0,
	PROCESS_SYSTEM_DPI_AWARE = 1,
	PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;
#endif

typedef BOOL(WINAPI* SETPROCESSDPIAWARE_T)(void);
typedef HRESULT(WINAPI* SETPROCESSDPIAWARENESS_T)(PROCESS_DPI_AWARENESS);



int WINAPI _tWinMain(HINSTANCE instance, HINSTANCE, LPTSTR cmdLine, int showCmd)
{
	HMODULE shcore = ::LoadLibraryA("Shcore.dll");
	SETPROCESSDPIAWARENESS_T SetProcessDpiAwareness = NULL;
	if (shcore) {
		SetProcessDpiAwareness = (SETPROCESSDPIAWARENESS_T)GetProcAddress(shcore, "SetProcessDpiAwareness");
		if (SetProcessDpiAwareness) {
			SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
		}
	}

#ifdef TESTMODE
	LibCC::g_pLog = new LibCC::Log(GetPathRelativeToApp(_T("screenie.log")), _Module.GetResourceInstance());
	TestMain();
	delete LibCC::g_pLog;
	return 0;
#endif

	// is this the second process started during a restart?
	bool bRestarting = false;
	for (int i = 0; i < __argc; i++)
	{
		if (wcscmp(__wargv[i], L"/restart") == 0)
			bRestarting = true;
	}

	// try to create a global mutex object. if it already exists, it means there's
	// another instance of this program already running, and we shouldn't run another.
	HANDLE instanceMutex = ::CreateMutex(NULL, TRUE, TEXT("ScreenieInstMutex"));

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		HWND screenieWindow = ::FindWindow(TEXT("ScreenieMainWnd"), TEXT("ScreenieWnd"));

		if (::IsWindow(screenieWindow))
		{
			::SendMessage(screenieWindow, CMainWindow::WM_SHOWSCREENIE, 0, 0);
			::CloseHandle(instanceMutex);

			// if there's another instance, it won't hurt us to return without closing
			// the mutex handle. the previous instance would be responsible for releasing it.

			return 0;
		}
	}

	// wait for 5 seconds. if we're the restarted process, this should have been more than enough time
	// for the original process to clean up and shut down. complain to the user, because it's probably hanging
	DWORD dwWaitResult = ::WaitForSingleObject(instanceMutex, 5000);
	switch (dwWaitResult)
	{
	case WAIT_TIMEOUT:
		{
			if (bRestarting)
			{
				// keep asking the user if he wants to try again, if he's so inclined
				bool bRetryWorked = false;
				while (!bRetryWorked &&
					::MessageBox(NULL, L"Screenie attempted to restart itself, but the original process is taking a long time to exit.\r\n"
					L"You may want to manually kill Screenie using Task Manager.", L"Screenie Error", MB_RETRYCANCEL | MB_ICONINFORMATION) == IDRETRY)
				{
					// well, let's try one more time, then
					dwWaitResult = ::WaitForSingleObject(instanceMutex, 5000);
					if (dwWaitResult != WAIT_TIMEOUT)
						bRetryWorked = true; // guess it worked
				}

				if (!bRetryWorked)
				{
					::CloseHandle(instanceMutex);
					return 0;
				}
			}
			break;
		}
	}

	// okay, now that we're done with that crap, actually run the app
	//////////////////////////////////////////////////////////////////////////

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
