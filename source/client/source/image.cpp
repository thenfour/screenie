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
		tstd::tstring extensionList = tstd::convert<tstd::tchar_t>(codecInfo->FilenameExtension);

		tstd::tstring::size_type dotPos = extensionList.find_first_of(_TT('.'));
		tstd::tstring::size_type semicolonPos = extensionList.find_first_of(_TT(';'));

		if (dotPrefix)
			return extensionList.substr(dotPos, semicolonPos - dotPos);

		return extensionList.substr(dotPos + 1, semicolonPos - dotPos - 1);
	}

	return tstd::tstring();
}

bool GetScreenshotBitmap(HBITMAP& bitmap, const RECT& rectToCopy, BOOL drawCursor)
{
	bool success = false;

	HDC screenDC = ::GetDC(NULL);
	HDC memoryDC = ::CreateCompatibleDC(screenDC);

	if (memoryDC != NULL)
	{
		bitmap = ::CreateCompatibleBitmap(screenDC,
			rectToCopy.right - rectToCopy.left,
			rectToCopy.bottom - rectToCopy.top);

		if (bitmap != NULL)
		{
			HGDIOBJ oldObj = ::SelectObject(memoryDC, reinterpret_cast<HGDIOBJ>(bitmap));

			::BitBlt(memoryDC, 0, 0,
				rectToCopy.right - rectToCopy.left,
				rectToCopy.bottom - rectToCopy.top,
				screenDC, rectToCopy.left, rectToCopy.top, SRCCOPY);

			if (drawCursor)
			{
				POINT cursorPos = { 0 };
				::GetCursorPos(&cursorPos);

				// let's see if the cursor would even be displayed in the
				// rectangle they gave us
				if (PtInRect(&rectToCopy, cursorPos))
				{
					CURSORINFO cursorInfo = { 0 };

					if (::GetCursorInfo(&cursorInfo))
						::DrawIcon(memoryDC, cursorPos.x, cursorPos.y, cursorInfo.hCursor);
				}
			}

			// we're all done! tell the troops they can go home.
			success = true;

			// select the previous object, thereby de-selecting our bitmap
			::SelectObject(memoryDC, oldObj);
		}

		::DeleteDC(memoryDC);
	}

	::ReleaseDC(NULL, screenDC);

	return success;
}

bool ScaleBitmap(util::shared_ptr<Gdiplus::Bitmap>& destination, Gdiplus::Bitmap& source, float scale)
{
	if (scale < 0)
		return false;

	int width = int(source.GetWidth() * scale);
	int height = int(source.GetHeight() * scale);

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
		std::wstring wideFilename = tstd::convert<wchar_t>(filename);

		if (image.Save(wideFilename.c_str(), &codecInfo->Clsid, NULL) == Gdiplus::Ok)
			return true;
	}

	return false;
}
