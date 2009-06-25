

#include "stdafx.hpp"
#include "CropDlg.hpp"

CAppModule _Module;

LibCC::Log* LibCC::g_pLog = 0;

int WINAPI _tWinMain(HINSTANCE instance, HINSTANCE, LPTSTR cmdLine, int showCmd)
{
	HRESULT hRes = ::CoInitialize(NULL);

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::Status status = Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	_Module.Init(NULL, instance);

	{
		LibCC::g_pLog = new LibCC::Log(L"editor.log", instance);
		CCropDlg x;
		x.DoModal();
		delete LibCC::g_pLog;
	}

	_Module.Term();

	Gdiplus::GdiplusShutdown(gdiplusToken);
	::CoUninitialize();

	return 0;
}
