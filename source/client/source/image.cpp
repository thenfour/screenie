//
// image.cpp - GDI+ and image-related routines
// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
//

#include "stdafx.hpp"

#include <algorithm>

#include "codec.hpp"
#include "image.hpp"
#include "utility.hpp"

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
		tstd::tstring extensionList = ToTstring(codecInfo->FilenameExtension);

		tstd::tstring::size_type dotPos = extensionList.find_first_of(_TT('.'));
		tstd::tstring::size_type semicolonPos = extensionList.find_first_of(_TT(';'));

		if (dotPrefix)
			return extensionList.substr(dotPos, semicolonPos - dotPos);

		return extensionList.substr(dotPos + 1, semicolonPos - dotPos - 1);
	}

	return tstd::tstring();
}




std::vector<DISPLAY_DEVICEW> GetDisplayDeviceNames() {
	std::vector<DISPLAY_DEVICEW> ret;
	for (int i = 0; i < 30000; ++i) { // insanity checking
		DISPLAY_DEVICEW dd = { 0 };
		dd.cb = sizeof(dd);
		if (!::EnumDisplayDevicesW(NULL, i, &dd, EDD_GET_DEVICE_INTERFACE_NAME)) {
			break;
		}
		ret.push_back(dd);
	}
	return ret;
}

std::vector<std::pair<DISPLAY_DEVICEW, DEVMODEW>> GetDisplaySettings() {
	auto dns = GetDisplayDeviceNames();
	std::vector<std::pair<DISPLAY_DEVICEW, DEVMODEW>> ret;
	for (auto& dn : dns) {
		DEVMODEW dm = { 0 };
		dm.dmSize = sizeof(dm);
		if (!EnumDisplaySettings(dn.DeviceName, ENUM_CURRENT_SETTINGS, &dm)) {
			return ret; // no idea why this is would happen.
		}
		ret.push_back(std::make_pair(dn, dm));
	}
	return ret;
}



bool GetCaptureAsBitmap(int x, int y, int width, int height, HBITMAP& bitmap, bool drawCursor)
{
	HDC screenDC = ::GetDC(0);
	auto memoryDC = ::CreateCompatibleDC(screenDC);

	// create a bitmap large enough for the virtual screen
	bitmap = CreateCompatibleBitmap(screenDC, width, height);

	HBITMAP hOld = (HBITMAP)SelectObject(memoryDC, bitmap);

	// fill with desktop background color
	RECT rc;
	SetRect(&rc, 0, 0, width, height);
	FillRect(memoryDC, &rc, (HBRUSH)(COLOR_DESKTOP + 1));

	BitBlt(memoryDC,
		0,
		0,
		width, height,
		screenDC,
		0, 0, SRCCOPY | CAPTUREBLT);

	if (drawCursor)
	{
		POINT cursorPos;
		GetCursorPos(&cursorPos);

		CURSORINFO cursorInfo = { 0 };
		cursorInfo.cbSize = sizeof(cursorInfo);

		if (::GetCursorInfo(&cursorInfo)) // Requires per-monitor DPI awareness.
		{
			ICONINFO ii = { 0 };
			GetIconInfo(cursorInfo.hCursor, &ii);
			DeleteObject(ii.hbmColor);
			DeleteObject(ii.hbmMask);
			::DrawIcon(memoryDC, cursorPos.x - ii.xHotspot, cursorPos.y - ii.yHotspot, cursorInfo.hCursor);
		}
	}

	SelectObject(memoryDC, hOld);

	DeleteDC(memoryDC);
	ReleaseDC(0, screenDC);

	return true;
}




bool GetDesktopWindowCaptureAsBitmap(HBITMAP& bitmap, bool drawCursor)
{
	auto screenInfo = GetDisplaySettings();
	int totalWidth = 0;
	int totalHeight = 0;
	for (auto& si : screenInfo) {
		//LibCC::g_pLog->Message(LibCC::Format("Monitor % size(%,%) @ pos(%,%)").qs(si.first.DeviceName).i(si.second.dmPelsWidth).i(si.second.dmPelsHeight).i(si.second.dmPosition.x).i(si.second.dmPosition.y));
		ATLASSERT(si.second.dmFields & DM_POSITION);
		ATLASSERT(si.second.dmFields & DM_PELSHEIGHT);
		ATLASSERT(si.second.dmFields & DM_PELSWIDTH);
		totalWidth = std::max(totalWidth, (int)(si.second.dmPosition.x + si.second.dmPelsWidth));
		totalHeight = std::max(totalHeight, (int)(si.second.dmPosition.y + si.second.dmPelsHeight));
	}

	return GetCaptureAsBitmap(0, 0, totalWidth, totalHeight, bitmap, drawCursor);
}

bool GetWindowCaptureAsBitmap(HWND hwnd, HBITMAP& bitmap, bool drawCursor)
{
	RECT rcWindow;

	// NB: This will ONLY give correct size if the application is per-monitor DPI-aware.
	// Otherwise it will return a value that's adapted to primary monitor or calling app etc.
	GetWindowRect(hwnd, &rcWindow);
	int width = rcWindow.right - rcWindow.left;
	int height = rcWindow.bottom - rcWindow.top;

	HDC screenDC = ::GetDC(hwnd);
	auto memoryDC = ::CreateCompatibleDC(screenDC);

	// create a bitmap large enough for the virtual screen
	bitmap = CreateCompatibleBitmap(screenDC, width, height);

	HBITMAP hOld = (HBITMAP)SelectObject(memoryDC, bitmap);

	// fill with desktop background color
	RECT rc;
	SetRect(&rc, 0, 0, width, height);
	FillRect(memoryDC, &rc, (HBRUSH)(COLOR_DESKTOP + 1));

	PrintWindow(hwnd, memoryDC, 0);

	if (drawCursor)
	{
		POINT cursorPos;
		GetCursorPos(&cursorPos);
		cursorPos.x -= rcWindow.left;
		cursorPos.y -= rcWindow.top;

		CURSORINFO cursorInfo = { 0 };
		cursorInfo.cbSize = sizeof(cursorInfo);

		if (::GetCursorInfo(&cursorInfo))
		{
			ICONINFO ii = { 0 };
			GetIconInfo(cursorInfo.hCursor, &ii);
			DeleteObject(ii.hbmColor);
			DeleteObject(ii.hbmMask);
			::DrawIcon(memoryDC, cursorPos.x - ii.xHotspot, cursorPos.y - ii.yHotspot, cursorInfo.hCursor);
		}
	}

	SelectObject(memoryDC, hOld);

	DeleteDC(memoryDC);
	ReleaseDC(hwnd, screenDC);

	return true;
}


bool GetScreenshotBitmap(HBITMAP& bitmap, BOOL AltPressed, BOOL drawCursor)
{
  if(AltPressed)
  {
	 return GetWindowCaptureAsBitmap(GetForegroundWindow(), bitmap, !!drawCursor);
  }
  return GetDesktopWindowCaptureAsBitmap(bitmap, !!drawCursor);
}

bool ScaleBitmap(std::shared_ptr<Gdiplus::Bitmap>& destination, Gdiplus::Bitmap& source, float scale)
{
	if (scale < 0)
		return false;

	int width = int(source.GetWidth() * scale);
	if(width == 0) width = 1;
	int height = int(source.GetHeight() * scale);
	if(height == 0) height = 1;

	std::shared_ptr<Gdiplus::Bitmap> bitmap(new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB));

	if (!bitmap)
		return false;

	Gdiplus::Graphics graphics(bitmap.get());

	if (graphics.DrawImage(&source, 0, 0, width, height) != Gdiplus::Ok)
		return false;

	destination = bitmap;

	return true;
}

bool ResizeBitmap(std::shared_ptr<Gdiplus::Bitmap>& destination, Gdiplus::Bitmap& source, int dimensionLimit)
{
	int maxDimension = std::max<int>(source.GetHeight(), source.GetWidth());

	// if this image doesn't even reach the limit, just return a clone of the bitmap.
	if (dimensionLimit >= maxDimension)
	{
		destination = std::shared_ptr<Gdiplus::Bitmap>(source.Clone(0, 0,
			source.GetWidth(), source.GetHeight(), PixelFormatDontCare));

		return true;
	}

	float scaleFactor = float(dimensionLimit) / maxDimension;
	return ScaleBitmap(destination, source, scaleFactor);
}

bool SaveImageToFile(Gdiplus::Image& image, const tstd::tstring& mimeType, const tstd::tstring& filename, ULONG quality)
{
	ImageCodecsEnum imageCodecs;
	Gdiplus::ImageCodecInfo* codecInfo = imageCodecs.GetCodecByMimeType(mimeType.c_str());

	if (codecInfo != NULL)
	{
		std::wstring wideFilename = LibCC::ToUTF16(filename);

		Gdiplus::EncoderParameters encoderParameters;
		encoderParameters.Count = 1;
		encoderParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
		encoderParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
		encoderParameters.Parameter[0].NumberOfValues = 1;
		encoderParameters.Parameter[0].Value = &quality;

		if (image.Save(wideFilename.c_str(), &codecInfo->Clsid, &encoderParameters) == Gdiplus::Ok)
			return true;
	}

	return false;
}

void DumpBitmap(Gdiplus::Bitmap& image, int x, int y)
{
  HDC dc = ::GetDC(0);
	{
		Gdiplus::Graphics screen(dc);
		screen.DrawImage(&image, x, y);
	}
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

