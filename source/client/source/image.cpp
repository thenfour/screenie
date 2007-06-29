//
// image.cpp - GDI+ and image-related routines
// Copyright (c) 2003 Carl Corcoran
// Copyright (c) 2005 Roger Clark
//

#include "stdafx.hpp"

#include "codec.hpp"
#include "image.hpp"

tstd::tstring GetGdiplusStatusString(Gdiplus::Status status)
{
	std::vector<tstd::tstring> name(22);

	name[0] = TEXT("Ok");
	name[1] = TEXT("GenericError");
	name[2] = TEXT("InvalidParameter");
	name[3] = TEXT("OutOfMemory");
	name[4] = TEXT("ObjectBusy");
	name[5] = TEXT("InsufficientBuffer");
	name[6] = TEXT("NotImplemented");
	name[7] = TEXT("Win32Error");
	name[8] = TEXT("WrongState");
	name[9] = TEXT("Aborted");
	name[10] = TEXT("FileNotFound");
	name[11] = TEXT("ValueOverflow");
	name[12] = TEXT("AccessDenied");
	name[13] = TEXT("UnknownImageFormat");
	name[14] = TEXT("FontFamilyNotFound");
	name[15] = TEXT("FontStyleNotFound");
	name[16] = TEXT("NotTrueTypeFont");
	name[17] = TEXT("UnsupportedGdiplusVersion");
	name[18] = TEXT("GdiplusNotInitialized");
	name[19] = TEXT("PropertyNotFound");
	name[20] = TEXT("PropertyNotSupported");
	name[21] = TEXT("ProfileNotFound");

	return name[status];
}

tstd::tstring GetImageCodecExtension(Gdiplus::ImageCodecInfo* codecInfo, bool dotPrefix)
{
	// each codec has a semicolon-delimited list of common extensions for a particular
	// image type. the first one, however, is the usual one -- that's the one we want.

	if (codecInfo != NULL)
	{
		tstd::tstring extensionList;
		LibCC::ConvertString(codecInfo->FilenameExtension, extensionList);

		tstd::tstring::size_type dotPos = extensionList.find_first_of(_TT('.'));
		tstd::tstring::size_type semicolonPos = extensionList.find_first_of(_TT(';'));

		if (dotPrefix)
			return extensionList.substr(dotPos, semicolonPos - dotPos);

		return extensionList.substr(dotPos + 1, semicolonPos - dotPos - 1);
	}

	return tstd::tstring();
}

struct GetVirtualScreenBitmap_Info
{
  std::vector<std::pair<tstd::tstring, CRect> > monitors;
  HDC memoryDC;
  // primary monitor's offset from upper-left virtual screen.  Primary monitor is always (0,0) even if it
  // is in the middle of the virtual screen.  so to eliminate negative coords, use this shift.
  int orgx;
  int orgy;// LOL
  int vwidth;
  int vheight;
};


BOOL CALLBACK GetDesktopWindowCaptureAsBitmap_MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT rcMon, LPARAM dwData)
{
  GetVirtualScreenBitmap_Info& info(*((GetVirtualScreenBitmap_Info*)dwData));
  MONITORINFOEX mi;
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(hMonitor, &mi);
  info.monitors.push_back(std::pair<tstd::tstring, CRect>(mi.szDevice, *rcMon));
  return TRUE;
}


bool GetDesktopWindowCaptureAsBitmap(HBITMAP& bitmap)
{
  GetVirtualScreenBitmap_Info info;

  HDC screenDC = ::GetDC(0);
  info.memoryDC = ::CreateCompatibleDC(screenDC);

  // create a bitmap large enough for the virtual screen
  info.vwidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  info.vheight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  bitmap = CreateCompatibleBitmap(screenDC, info.vwidth, info.vheight);

  HBITMAP hOld = (HBITMAP)SelectObject(info.memoryDC, bitmap);

  // fill with desktop background color
  RECT rc;
  SetRect(&rc, 0, 0, info.vwidth, info.vheight);
  FillRect(info.memoryDC, &rc, (HBRUSH)(COLOR_DESKTOP+1));

  info.orgx = GetSystemMetrics(SM_XVIRTUALSCREEN);
  info.orgy = GetSystemMetrics(SM_YVIRTUALSCREEN);

  // get monitor info
  EnumDisplayMonitors(NULL, NULL, GetDesktopWindowCaptureAsBitmap_MonitorEnumProc, (LPARAM)&info);  

  int i = 0;
  for(std::vector<std::pair<tstd::tstring, CRect> >::iterator it = info.monitors.begin(); it != info.monitors.end(); ++ it)
  {
    HDC dcSource = CreateDC(0, it->first.c_str(), 0, 0);
    CRect& rcScreen = it->second;

    BitBlt(info.memoryDC,
      rcScreen.left - info.orgx,
      rcScreen.top - info.orgy,
      rcScreen.Width(),
      rcScreen.Height(),
      dcSource,
      0, 0, SRCCOPY);

    DeleteDC(dcSource);
    i++;
  }

  SelectObject(info.memoryDC, hOld);

  DeleteDC(info.memoryDC);
  ReleaseDC(0, screenDC);

  return true;
}

bool GetScreenshotBitmap(HBITMAP& bitmap, BOOL AltPressed, BOOL drawCursor)
{
	bool success = false;

  // get this stuff immediately
  POINT cursorPos = { 0 };
  ::GetCursorPos(&cursorPos);
  // adjust cursor pos so it's always positive
  cursorPos.x -= GetSystemMetrics(SM_XVIRTUALSCREEN);
  cursorPos.y -= GetSystemMetrics(SM_YVIRTUALSCREEN);

  HWND hWnd = GetForegroundWindow();

  GetDesktopWindowCaptureAsBitmap(bitmap);

  if(AltPressed)
  {
    // crop it down to the window.
    HBITMAP full = bitmap;
    RECT rc;
    GetWindowRect(hWnd, &rc);
    rc.left -= GetSystemMetrics(SM_XVIRTUALSCREEN);
    rc.right -= GetSystemMetrics(SM_XVIRTUALSCREEN);
    rc.bottom -= GetSystemMetrics(SM_YVIRTUALSCREEN);
    rc.top -= GetSystemMetrics(SM_YVIRTUALSCREEN);

	WINDOWPLACEMENT wndpl = {sizeof(WINDOWPLACEMENT)};
	GetWindowPlacement(hWnd, &wndpl);
	
	if(wndpl.showCmd == SW_SHOWMAXIMIZED)
	{
		// maximised windows have magic non-visible borders, trim them off

		WINDOWINFO wndinf = {sizeof(WINDOWINFO)};
		GetWindowInfo(hWnd, &wndinf);

		rc.left += wndinf.cxWindowBorders;
		rc.right -= wndinf.cxWindowBorders;
		rc.top += wndinf.cyWindowBorders;
		rc.bottom -= wndinf.cyWindowBorders;
	}

    HDC screenDC = ::GetDC(0);
    HDC sourceDC = ::CreateCompatibleDC(screenDC);
    HBITMAP sourceOld = (HBITMAP)SelectObject(sourceDC, full);
    HDC destDC = ::CreateCompatibleDC(screenDC);
    bitmap = CreateCompatibleBitmap(screenDC, rc.right - rc.left, rc.bottom - rc.top);
    HBITMAP destOld = (HBITMAP)SelectObject(destDC, bitmap);

    // do the copy.
    BitBlt(destDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top, sourceDC, rc.left, rc.top, SRCCOPY);

    SelectObject(destDC, destOld);
    DeleteDC(destDC);
    SelectObject(sourceDC, sourceOld);
    DeleteDC(sourceDC);

    ReleaseDC(0, screenDC);

    DeleteObject(full);

    // adjust mouse cursor position.
    cursorPos.x -= rc.left;
    cursorPos.y -= rc.top;
  }

  if (drawCursor)
  {
    BITMAP bm = {0};
    GetObject(bitmap, sizeof(bm), &bm);

    // let's see if the cursor would even be displayed in the
    // rectangle they gave us
    if(cursorPos.x >= 0 && cursorPos.y >= 0 && cursorPos.y < bm.bmHeight && cursorPos.x < bm.bmWidth)
    {
	    HDC screenDC = ::GetDC(0);
	    HDC memoryDC = ::CreateCompatibleDC(screenDC);
      ReleaseDC(0, screenDC);
      HBITMAP hOld = (HBITMAP)SelectObject(memoryDC, bitmap);

      CURSORINFO cursorInfo = { 0 };
      cursorInfo.cbSize = sizeof(cursorInfo);

      if (::GetCursorInfo(&cursorInfo))
      {
        ICONINFO ii = {0};
        GetIconInfo(cursorInfo.hCursor, &ii);
        DeleteObject(ii.hbmColor);
        DeleteObject(ii.hbmMask);
        ::DrawIcon(memoryDC, cursorPos.x - ii.xHotspot, cursorPos.y - ii.yHotspot, cursorInfo.hCursor);
      }

      SelectObject(memoryDC, hOld);
      DeleteDC(memoryDC);
    }
  }

	success = true;

	return success;
}

bool ScaleBitmap(util::shared_ptr<Gdiplus::Bitmap>& destination, Gdiplus::Bitmap& source, float scale)
{
	if (scale < 0)
		return false;

	int width = int(source.GetWidth() * scale);
	if(width == 0) width = 1;
	int height = int(source.GetHeight() * scale);
	if(height == 0) height = 1;

	util::shared_ptr<Gdiplus::Bitmap> bitmap(new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB));

	if (!bitmap)
		return false;

	Gdiplus::Graphics graphics(bitmap.get());

	if (graphics.DrawImage(&source, 0, 0, width, height) != Gdiplus::Ok)
		return false;

	destination = bitmap;

	return true;
}

bool ResizeBitmap(util::shared_ptr<Gdiplus::Bitmap>& destination, Gdiplus::Bitmap& source, int dimensionLimit)
{
	int maxDimension = std::max<int>(source.GetHeight(), source.GetWidth());

	// if this image doesn't even reach the limit, just return a clone of the bitmap.
	if (dimensionLimit >= maxDimension)
	{
		destination = util::shared_ptr<Gdiplus::Bitmap>(source.Clone(0, 0,
			source.GetWidth(), source.GetHeight(), PixelFormatDontCare));

		return true;
	}

	float scaleFactor = float(dimensionLimit) / maxDimension;
	return ScaleBitmap(destination, source, scaleFactor);
}

bool SaveImageToFile(Gdiplus::Image& image, const tstd::tstring& mimeType, const tstd::tstring& filename)
{
	ImageCodecsEnum imageCodecs;
	Gdiplus::ImageCodecInfo* codecInfo = imageCodecs.GetCodecByMimeType(mimeType.c_str());

	if (codecInfo != NULL)
	{
		std::wstring wideFilename = LibCC::ToUnicode(filename);

		if (image.Save(wideFilename.c_str(), &codecInfo->Clsid, NULL) == Gdiplus::Ok)
			return true;
	}

	return false;
}

void DumpBitmap(Gdiplus::Bitmap& image, int x, int y)
{
  // draw that damn bitmap to the screen.
  HDC dc = ::GetDC(0);
  HDC dcc = CreateCompatibleDC(dc);
  HBITMAP himg;
  image.GetHBITMAP(0, &himg);
  HBITMAP hOld = (HBITMAP)SelectObject(dcc, himg);
  StretchBlt(dc, x, y, image.GetWidth(), image.GetHeight(), dcc, 0, 0, image.GetWidth(), image.GetHeight(), SRCCOPY);
  SelectObject(dcc, hOld);
  DeleteDC(dcc);
  DeleteObject(himg);
  ::ReleaseDC(0,dc);
}

void DumpBitmap(HBITMAP himg, int x, int y)
{
  // draw that damn bitmap to the screen.
  BITMAP bi;
  GetObject(himg, sizeof(bi), &bi);

  HDC dc = ::GetDC(0);
  HDC dcc = CreateCompatibleDC(dc);
  HBITMAP hOld = (HBITMAP)SelectObject(dcc, himg);
  StretchBlt(dc, x, y, bi.bmWidth, bi.bmHeight, dcc, 0, 0, bi.bmWidth, bi.bmHeight, SRCCOPY);
  SelectObject(dcc, hOld);
  DeleteDC(dcc);
  ::ReleaseDC(0,dc);
}

void DumpIcon(HICON img, int x, int y)
{
  HDC dc = ::GetDC(0);
  ICONINFO ii;
  GetIconInfo(img, &ii);

  BITMAP bi;
  GetObject(ii.hbmColor, sizeof(bi), &bi);

  DumpBitmap(ii.hbmColor, x + bi.bmWidth, y);
  DeleteObject(ii.hbmColor);
  DumpBitmap(ii.hbmMask, x + bi.bmWidth + bi.bmWidth, y);
  DeleteObject(ii.hbmMask);

  DrawIcon(dc, x, y, img);
  ::ReleaseDC(0,dc);
}


bool GdiplusBitmapToAnimBitmap(Gdiplus::BitmapPtr src, AnimBitmap<32>& dest)
{
	ATLASSERT(!"I can't figure out how to get this to work.");
	//if(!dest.SetSize(src->GetWidth(), src->GetHeight()))
	//	return false;

	//// the easy / fast way. BUT it doesn't fucking work. I think Gdiplus::Graphics() doesn't like the DC i pass in
	////Gdiplus::Graphics* gfx = new Gdiplus::Graphics(dest.GetDC());
	////Gdiplus::Status s = gfx->GetLastStatus();
	////s = gfx->DrawImage(src.get(), 0, 0);
	////delete gfx;

 // HBITMAP himg;
 // src->GetHBITMAP(0, &himg);

	//HDC dcDesktop = GetDC(0);
	//HDC dcc = CreateCompatibleDC(dcDesktop);
	//ReleaseDC(0, dcDesktop);

	//HBITMAP hOld = (HBITMAP)SelectObject(dcc, himg);

	//dest.BlitFrom(dcc, 0, 0, src->GetWidth(), src->GetHeight());

	//SelectObject(dcc, hOld);
 // DeleteDC(dcc);
 // DeleteObject(himg);

	return true;
}

