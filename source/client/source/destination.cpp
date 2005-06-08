//
// destination.cpp - actual processing of FTP destinations
// Copyright (c) 2003 Carl Corcoran
// Copyright (c) 2005 Roger Clark
//

#include "stdafx.hpp"
#include "resource.h"

// ui
#include "ScreenshotDestination.hpp"
#include "StatusDlg.hpp"

// general
#include "clipboard.hpp"
#include "codec.hpp"
#include "destination.hpp"
#include "image.hpp"
#include "internet.hpp"
#include "path.hpp"
#include "utility.hpp"

bool GetTransformedScreenshot(const ScreenshotDestination::Image& options,
	util::shared_ptr<Gdiplus::Bitmap> screenshot, util::shared_ptr<Gdiplus::Bitmap>& transformed)
{
	if (!screenshot)
		return false;

	if (options.scaleType == ScreenshotDestination::SCALE_SCALETOPERCENT)
		return ScaleBitmap(transformed, *screenshot, options.scalePercent / 100.0f);
	else if (options.scaleType == ScreenshotDestination::SCALE_LIMITDIMENSIONS)
		return ResizeBitmap(transformed, *screenshot, options.maxDimension);

	// just point 'transformed' back to the original screenshot, as long
	// as it's there. we're not doing any scaling. shared_ptr takes care
	// of the reference counting, so we don't need to manage its lifetime.

	transformed = screenshot;

	return true;
}

// FIXME: these WinInet calls block, and they stall processing. multithread the app
// (or at least this portion) to allow for GUI responsiveness

bool ProcessFtpDestination(HWND hwnd, StatusWindow& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image, bool& usedClipboard)
{
  LPARAM msgid = status.CreateProgressMessage(destination.general.name, _T(""));

	WinInetHandle internet = ::InternetOpen("Screenie v1.0", INTERNET_OPEN_TYPE_DIRECT,
		NULL, NULL, 0);

	if (internet.handle == NULL)
	{
		status.PrintMessage(StatusWindow::MSG_ERROR, destination.general.name,
			TEXT("FTP: Can't initialize internet connection"));

		return false;
	}

	WinInetHandle ftp = ::InternetConnect(internet.handle,
		destination.ftp.hostname.c_str(), destination.ftp.port,
		destination.ftp.username.c_str(), destination.ftp.DecryptPassword().c_str(),
		INTERNET_SERVICE_FTP, 0, 0);

	if (ftp.handle == NULL)
	{
		status.PrintMessage(StatusWindow::MSG_ERROR, destination.general.name,
			LibCC::Format(TEXT("FTP: Can't connect to server: %")).s(GetWinInetErrorString()).Str());

		return false;
	}

	util::shared_ptr<Gdiplus::Bitmap> transformedImage;
	if (!GetTransformedScreenshot(destination.image, image, transformedImage))
	{
		status.PrintMessage(StatusWindow::MSG_ERROR, destination.general.name,
			TEXT("FTP: Can't resize screenshot!"));

		return false;
	}

	// before we can upload the image, we need to save it to a temporary file.
	tstd::tstring temporaryFilename = GetUniqueTemporaryFilename();
	if (!SaveImageToFile(*transformedImage, destination.general.imageFormat, temporaryFilename))
	{
		status.PrintMessage(StatusWindow::MSG_ERROR, destination.general.name,
			TEXT("FTP: Can't save image to temporary file!"));

		return false;
	}

	if (!::FtpSetCurrentDirectory(ftp.handle, destination.ftp.remotePath.c_str()))
	{
		status.PrintMessage(StatusWindow::MSG_ERROR, destination.general.name,
			LibCC::Format(TEXT("FTP: Can't navigate to remote path \"%\" (%)")).s(destination.ftp.remotePath).s(GetWinInetErrorString()).Str());

		return false;
	}

	// format the filename based on the current time

	SYSTEMTIME systemTime = { 0 };
	::GetSystemTime(&systemTime);

	tstd::tstring filename = FormatFilename(systemTime, destination.general.filenameFormat);

	if (!::FtpPutFile(ftp.handle, temporaryFilename.c_str(), filename.c_str(),
		FTP_TRANSFER_TYPE_BINARY, 0))
	{
		status.PrintMessage(StatusWindow::MSG_ERROR, destination.general.name,
			TEXT("FTP: Can't upload image to server!"));

		return false;
	}

	status.PrintMessage(StatusWindow::MSG_INFO, destination.general.name,
		TEXT("FTP: Uploaded screenshot to server."));

	if (!destination.ftp.resultURL.empty())
	{
		tstd::tstring url = LibCC::Format(TEXT("%%")).s(destination.ftp.resultURL).s(filename).Str();
		status.PrintMessage(StatusWindow::MSG_INFO, destination.general.name, url);

		if (destination.ftp.copyURL)
		{
      if(usedClipboard)
      {
        status.PrintMessage(StatusWindow::MSG_WARNING, destination.general.name, _T("Warning: Overwriting clipboard contents"));
      }
			try
			{
				Clipboard(hwnd).SetText(url);
        status.PrintMessage(StatusWindow::MSG_INFO, destination.general.name,
          LibCC::Format("Copied URL to clipboard %").qs(url).Str());
        usedClipboard = true;
			}
			catch (const Win32Exception& excp)
			{
				status.PrintMessage(StatusWindow::MSG_ERROR, destination.general.name,
					LibCC::Format(TEXT("Can't copy text to clipboard: %")).s(excp.What()).Str());
			}
		}
	}

	return true;
}

bool ProcessFileDestination(HWND hwnd, StatusWindow& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image, bool& usedClipboard)
{
	bool success = false;

	// let's see if the directory they want us to save to even exists.
	if (!::PathFileExists(destination.general.path.c_str()))
	{
		status.PrintMessage(StatusWindow::MSG_ERROR, destination.general.name,
			LibCC::Format(TEXT("File: folder \"%\" doesn't exist")).s(destination.general.path).Str());

		return false;
	}

	// try to get the transformed screenshot.
	util::shared_ptr<Gdiplus::Bitmap> transformedScreenshot;
	if (!GetTransformedScreenshot(destination.image, image, transformedScreenshot))
	{
		status.PrintMessage(StatusWindow::MSG_ERROR, destination.general.name,
			LibCC::Format(TEXT("File: can't get screenshot data!")).s(destination.general.path).Str());

		return false;
	}

	// let's get the filename and format it.

	SYSTEMTIME systemTime = { 0 };
	::GetSystemTime(&systemTime);

	tstd::tstring filename = FormatFilename(systemTime, destination.general.filenameFormat);
	tstd::tstring fullPath = LibCC::Format(TEXT("%\\%")).s(destination.general.path).s(filename).Str();

	// do the deed
	if (SaveImageToFile(*transformedScreenshot, destination.general.imageFormat, fullPath))
	{
		status.PrintMessage(StatusWindow::MSG_INFO, destination.general.name,
			LibCC::Format(TEXT("File: saved image to %\\%")).s(destination.general.path).s(filename).Str());
	}
	else
	{
		status.PrintMessage(StatusWindow::MSG_ERROR, destination.general.name,
			TEXT("File: couldn't save image to disk"));

		return false;
	}

	return success;
}

HBITMAP DuplicateScreenshotBitmap(HBITMAP sourceBitmap)
{
	HDC desktopDC = ::GetDC(NULL);

	BITMAP bmp = { 0 };
	if (!::GetObject(sourceBitmap, sizeof(bmp), &bmp))
		return NULL;

	HDC destDC = ::CreateCompatibleDC(desktopDC);
	HBITMAP destBitmap = ::CreateCompatibleBitmap(desktopDC, bmp.bmWidth, bmp.bmHeight);
	HGDIOBJ destOldObj = ::SelectObject(destDC, destBitmap);

	HDC sourceDC = ::CreateCompatibleDC(desktopDC);
	HGDIOBJ sourceOldObj = ::SelectObject(sourceDC, sourceBitmap);

	::BitBlt(destDC, 0, 0, bmp.bmWidth, bmp.bmHeight, sourceDC, 0, 0, SRCCOPY);

	::SelectObject(sourceDC, sourceOldObj);
	::DeleteDC(sourceDC);

	::SelectObject(destDC, destOldObj);
	::DeleteDC(destDC);

	return destBitmap;
}

bool ProcessClipboardDestination(HWND hwnd, StatusWindow& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image, bool& usedClipboard)
{
  if(usedClipboard)
  {
    status.PrintMessage(StatusWindow::MSG_WARNING, destination.general.name, _T("Warning: Overwriting clipboard contents"));
  }

	if (!::OpenClipboard(hwnd))
	{
		status.PrintMessage(StatusWindow::MSG_ERROR, destination.general.name,
			TEXT("Clipboard: Can't get clipboard access!"));

		return false;
	}

	if (!::EmptyClipboard())
	{
		status.PrintMessage(StatusWindow::MSG_ERROR, destination.general.name,
			TEXT("Clipboard: Can't empty previous clipboard contents!"));

		return false;
	}

	bool success = false;

	// try to get the transformed screenshot.
	util::shared_ptr<Gdiplus::Bitmap> transformedScreenshot;
	if (GetTransformedScreenshot(destination.image, image, transformedScreenshot))
	{
		HBITMAP clipboardBitmap = NULL;
		if (transformedScreenshot->GetHBITMAP(Gdiplus::Color(0,0,0), &clipboardBitmap) == Gdiplus::Ok)
		{
			// the system will take care of deleting this bitmap.
			HBITMAP bitmapCopy = DuplicateScreenshotBitmap(clipboardBitmap);

			try
			{
				Clipboard(hwnd).SetBitmap(bitmapCopy);
        status.PrintMessage(StatusWindow::MSG_CHECK, destination.general.name,
          _T("Copied image to clipboard"));
        usedClipboard = true;
			}
			catch (const Win32Exception& excp)
			{
				status.PrintMessage(StatusWindow::MSG_ERROR, destination.general.name,
					LibCC::Format(TEXT("Clipboard: Can't copy image: %")).s(excp.What()).Str());
			}
		}
		else
		{
			status.PrintMessage(StatusWindow::MSG_ERROR, destination.general.name,
				TEXT("Clipboard: Can't get clipboard-friendly image data!"));
		}
	}
	else
	{
		status.PrintMessage(StatusWindow::MSG_ERROR, destination.general.name,
			LibCC::Format(TEXT("File: can't get screenshot data!")).s(destination.general.path).Str());
	}

	::CloseClipboard();

	return true;
}

bool ProcessEmailDestination(HWND hwnd, StatusWindow& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image, bool& usedClipboard)
{
	return true;
}

bool ProcessDestination(HWND hwnd, StatusWindow& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image, bool& usedClipboard)
{
	switch (destination.general.type)
	{
		case ScreenshotDestination::TYPE_FILE:
			return ProcessFileDestination(hwnd, status, destination, image, usedClipboard);
			break;
		case ScreenshotDestination::TYPE_FTP:
			return ProcessFtpDestination(hwnd, status, destination, image, usedClipboard);
			break;
		case ScreenshotDestination::TYPE_CLIPBOARD:
			return ProcessClipboardDestination(hwnd, status, destination, image, usedClipboard);
			break;
		case ScreenshotDestination::TYPE_EMAIL:
			return ProcessEmailDestination(hwnd, status, destination, image, usedClipboard);
			break;
	}

	return false;
}