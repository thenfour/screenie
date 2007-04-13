
#include "stdafx.hpp"
#include "animbitmap.h"
#include "viewport.h"
#include "libcc/timer.h"
#include "libcc/log.h"
#include "libcc/stringutil.h"

#include "image.hpp"
#include "cropdlg.hpp"

/*
	Performance testing of the FillCheckerPattern function.
	I did a few simple optimizations and basically in the worst case, execution time is cut 7%.
	it's cut dramatically though for most cases - probably about 60%.

	original:                                4.376 seconds
	using GDI functions:                     4.262 seconds
	same, but this time caching GDI objects: 4.186 seconds
	now with tiny exclusion rectangle:       4.153 seconds
	with medium sized exclusion rectangle:   1.836 seconds

	So, code optimizations did not really help much. The biggest benefit will be eliminating redundant operations.
*/

void TestMain()
{
	//LibCC::g_pLog = new LibCC::Log("testlog.txt", GetModuleHandle(0));

	//AnimBitmap<32> x;
	//x.SetSize(1000, 1000);

	//LibCC::Timer t;
	//t.Tick();
	//for(int i = 0; i < 1000; i ++)
	//{
	//	CRect rc(100, 100, 901, 901);
	//	x.FillCheckerPattern(rc);
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

	// just startup with the cropping dialog.
	//Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	//ULONG_PTR gdiplusToken;
	//Gdiplus::Status status = Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	//{
	//	HBITMAP hbm;
	//	GetScreenshotBitmap(hbm, FALSE, FALSE);
	//	util::shared_ptr<Gdiplus::Bitmap> screenshot(new Gdiplus::Bitmap(hbm, NULL));
	//	CCropDlg cropDialog(screenshot, ScreenshotOptions());
	//	cropDialog.DoModal();
	//}
	//Gdiplus::GdiplusShutdown(gdiplusToken);
}

