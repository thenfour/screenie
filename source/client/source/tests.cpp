// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark

#include "stdafx.hpp"
#include "animbitmap.h"
#include "viewport.h"
#include "libcc/timer.hpp"
#include "libcc/log.hpp"
#include "libcc/stringutil.hpp"

#include "image.hpp"
#include "cropdlg.hpp"

/*
	Performance testing of the FillCheckerPattern function.
	I did a few simple optimizations and basically in the worst case, execution time is cut 7%.
	it's cut dramatically though for most cases - probably about 60%.

	original:                                4.376 seconds / 228 fps
	using GDI functions:                     4.262 seconds / 234 fps
	same, but this time caching GDI objects: 4.186 seconds / 238 fps
	now with tiny exclusion rectangle:       4.153 seconds / 240 fps
	with medium sized exclusion rectangle:   1.836 seconds / 544 fps (100,100,901,901)
	ok this time caching the entire bitmap:  2.677 seconds / 373 fps <-- this was also done on a faster processor though....
	same thing with big exclusion rectangle: 0.512 seconds / 1953 fps -- obviously the clear winner.
*/

void TestMain()
{
	//LibCC::g_pLog = new LibCC::Log("testlog.txt", GetModuleHandle(0));

	//AnimBitmap<32> x;
	//AnimBitmap<32> y;
	//x.SetSize(1000, 1000);
	//y.SetSize(1000, 1000);
	//x.FillCheckerPattern();

	//LibCC::Timer t;
	//t.Tick();
	//for(int i = 0; i < 1000; i ++)
	//{
	//	CRect rc(100, 100, 101, 101);
	//	//x.FillCheckerPattern(rc);
	//	//x.Blit(y, 0, 0);
	//	x.ExclusionBlit(y, rc);
	//}
	//t.Tick();
	//LibCC::g_pLog->Message(LibCC::Format("FillCheckerPattern: % seconds").d<3>(t.GetLastDelta()).CStr());

	//HDC dcScreen = GetDC(0);
	//x.Blit(dcScreen, 0, 0);
	//ReleaseDC(0, dcScreen);

	//MessageBoxW(0, L"", L"", MB_OK);

	//Viewport v;
	//v.SetZoomFactor(1);
	//v.SetViewOrigin(PointF(0,0));
	//v.SetImageOrigin(PointF(0,0));

	//PointF f;
	//f = v.ViewToImage(PointF(-1,-1));
	//f = v.ImageToView(f);
	//f = v.ViewToImage(PointF(0,0));
	//f = v.ImageToView(f);
	//f = v.ViewToImage(PointF(1,1));
	//f = v.ImageToView(f);

	//f = v.ViewToImage(PointF(-.5,-.5));
	//f = v.ImageToView(f);
	//f = v.ViewToImage(PointF(.5,.5));
	//f = v.ImageToView(f);

	//// setting zoom factor to .5 should make view coords have a more dramatic effect on Image coords.
	//// so -1 View = -2 Image, etc.
	//v.SetZoomFactor(.5);
	//f = v.ViewToImage(PointF(-1,-1));
	//f = v.ImageToView(f);
	//f = v.ViewToImage(PointF(0,0));
	//f = v.ImageToView(f);
	//f = v.ViewToImage(PointF(1,1));
	//f = v.ImageToView(f);

	//f = v.ViewToImage(PointF(-.5,-.5));
	//f = v.ImageToView(f);
	//f = v.ViewToImage(PointF(.5,.5));
	//f = v.ImageToView(f);

	//v.SetViewOrigin(PointF(.1,.1));
	//f = v.ViewToImage(PointF(-.5,-.5));
	//f = v.ImageToView(f);
	//f = v.ViewToImage(PointF(0,0));
	//f = v.ImageToView(f);
	//f = v.ViewToImage(PointF(.5,.5));
	//f = v.ImageToView(f);
	//f = v.ViewToImage(PointF(1,1));
	//f = v.ImageToView(f);

	//// FormatSize()
	//std::wstring s;
	//s = FormatSize(0);
	//s = FormatSize(1);
	//s = FormatSize(10);
	//s = FormatSize(99);
	//s = FormatSize(100);
	//s = FormatSize(999);
	//s = FormatSize(1000);
	//s = FormatSize(1023);
	//s = FormatSize(1024);
	//s = FormatSize(1025);
	//s = FormatSize(0xffffffff);
	//s = FormatSize(0x400);
	//s = FormatSize(0x800);
	//s = FormatSize(0x801);
	//s = FormatSize(0x4000);
	//s = FormatSize(0x40000);
	//s = FormatSize(0x400000);
	//s = FormatSize(0x4000000);

	// just startup with the cropping dialog.
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	Gdiplus::Status status = Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	{
		HBITMAP hbm;
		GetScreenshotBitmap(hbm, FALSE, FALSE);
		std::shared_ptr<Gdiplus::Bitmap> screenshot(new Gdiplus::Bitmap(hbm, NULL));
		ScreenshotOptions options;
		options.LoadSettings();
		CCropDlg cropDialog(screenshot, options);
		cropDialog.DoModal();
	}
	Gdiplus::GdiplusShutdown(gdiplusToken);

}

