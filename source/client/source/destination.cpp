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
#include "BuyDlg.h"

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


struct ProcessFtpDestination_Info
{
  AsyncStatusWindow* status;
  LPARAM msgid;
};

bool ProcessFtpDestination_ProgressProc(DWORD completed, DWORD total, void* pUser)
{
  ProcessFtpDestination_Info& info(*((ProcessFtpDestination_Info*)pUser));
  info.status->AsyncMessageSetProgress(info.msgid, static_cast<int>(completed), static_cast<int>(total));
  return true;
}

bool ProcessFtpDestination(HWND hwnd, AsyncStatusWindow& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image, bool& usedClipboard)
{
  LPARAM msgid = status.AsyncCreateMessage(AsyncStatusWindow::MSG_PROGRESS, AsyncStatusWindow::ITEM_FTP, destination.general.name, _T("Initiating FTP transfer"));
  status.AsyncMessageSetProgress(msgid, 0, 1);// set it to 0%

	util::shared_ptr<Gdiplus::Bitmap> transformedImage;
	if (!GetTransformedScreenshot(destination.image, image, transformedImage))
	{
    status.AsyncMessageSetText(msgid, TEXT("FTP: Can't resize screenshot!"));
    status.AsyncMessageSetIcon(msgid, AsyncStatusWindow::MSG_ERROR);
		return false;
	}

	// before we can upload the image, we need to save it to a temporary file.
	tstd::tstring temporaryFilename = GetUniqueTemporaryFilename();
	if (!SaveImageToFile(*transformedImage, destination.general.imageFormat, temporaryFilename))
	{
    status.AsyncMessageSetText(msgid, TEXT("FTP: Can't save image to temporary file!"));
    status.AsyncMessageSetIcon(msgid, AsyncStatusWindow::MSG_ERROR);
		return false;
	}

	// format the destination filename based on the current time
	SYSTEMTIME systemTime = { 0 };
  destination.GetNowBasedOnTimeSettings(systemTime);
	tstd::tstring remoteFileName = FormatFilename(systemTime, destination.general.filenameFormat);

  LibCC::Result r;
  // set up info struct to pass to the progress proc.
  ProcessFtpDestination_Info info;
  info.msgid = msgid;
  info.status = &status;
  if(!(r = UploadFTPFile(destination, temporaryFilename, remoteFileName, 4000, ProcessFtpDestination_ProgressProc, &info)))
  {
    status.AsyncMessageSetIcon(msgid, AsyncStatusWindow::MSG_ERROR);
    status.AsyncMessageSetText(msgid, r.str());
    return false;
  }

  // delete the temp file
  DeleteFile(temporaryFilename.c_str());

  status.AsyncMessageSetText(msgid, TEXT("Upload complete."));
  status.AsyncMessageSetIcon(msgid, AsyncStatusWindow::MSG_CHECK);

	if (!destination.ftp.resultURL.empty())
	{
    tstd::tstring url = LibCC::Format(TEXT("%%")).s(destination.ftp.resultURL).s(remoteFileName).Str();
    status.AsyncMessageSetText(msgid, LibCC::Format("Uploaded to: %").s(url).Str());
    status.AsyncMessageSetURL(msgid, url);

		if (destination.ftp.copyURL)
		{
      if(usedClipboard)
      {
        status.AsyncCreateMessage(AsyncStatusWindow::MSG_WARNING, AsyncStatusWindow::ITEM_GENERAL, destination.general.name, _T("Warning: Overwriting clipboard contents"));
      }

			try
			{
				Clipboard(hwnd).SetText(url);
        status.AsyncCreateMessage(AsyncStatusWindow::MSG_INFO, AsyncStatusWindow::ITEM_GENERAL, destination.general.name,
          LibCC::Format("Copied URL to clipboard %").qs(url).Str());
        usedClipboard = true;
			}
			catch (const Win32Exception& excp)
			{
				status.AsyncCreateMessage(AsyncStatusWindow::MSG_ERROR, AsyncStatusWindow::ITEM_GENERAL, destination.general.name,
					LibCC::Format(TEXT("Can't copy text to clipboard: %")).s(excp.What()).Str());
			}
		}
	}
	return true;
}

bool ProcessFileDestination(HWND hwnd, AsyncStatusWindow& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image, bool& usedClipboard)
{
	bool success = false;

	// let's see if the directory they want us to save to even exists.
	if (!::PathFileExists(destination.general.path.c_str()))
	{
    status.AsyncCreateMessage(AsyncStatusWindow::MSG_ERROR, AsyncStatusWindow::ITEM_GENERAL, destination.general.name,
			LibCC::Format(TEXT("File: folder \"%\" doesn't exist")).s(destination.general.path).Str());
		return false;
	}

	// try to get the transformed screenshot.
	util::shared_ptr<Gdiplus::Bitmap> transformedScreenshot;
	if (!GetTransformedScreenshot(destination.image, image, transformedScreenshot))
	{
		status.AsyncCreateMessage(AsyncStatusWindow::MSG_ERROR, AsyncStatusWindow::ITEM_GENERAL, destination.general.name,
			LibCC::Format(TEXT("File: can't get screenshot data!")).s(destination.general.path).Str());
		return false;
	}

	// let's get the filename and format it.

	SYSTEMTIME systemTime = { 0 };
  destination.GetNowBasedOnTimeSettings(systemTime);

	tstd::tstring filename = FormatFilename(systemTime, destination.general.filenameFormat);
	tstd::tstring fullPath = LibCC::Format(TEXT("%\\%")).s(destination.general.path).s(filename).Str();

	// do the deed
	if (SaveImageToFile(*transformedScreenshot, destination.general.imageFormat, fullPath))
	{
    status.AsyncCreateMessage(AsyncStatusWindow::MSG_CHECK, AsyncStatusWindow::ITEM_FILE, destination.general.name,
			LibCC::Format(TEXT("File: saved image to %")).qs(fullPath).Str(), fullPath);
	}
	else
	{
    status.AsyncCreateMessage(AsyncStatusWindow::MSG_ERROR, AsyncStatusWindow::ITEM_GENERAL, destination.general.name,
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

bool ProcessClipboardDestination(HWND hwnd, AsyncStatusWindow& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image, bool& usedClipboard)
{
  if(usedClipboard)
  {
    status.AsyncCreateMessage(AsyncStatusWindow::MSG_WARNING, AsyncStatusWindow::ITEM_GENERAL, destination.general.name, _T("Warning: Overwriting clipboard contents"));
  }

	if (!::OpenClipboard(hwnd))
	{
		status.AsyncCreateMessage(AsyncStatusWindow::MSG_ERROR, AsyncStatusWindow::ITEM_GENERAL, destination.general.name,
			TEXT("Clipboard: Can't get clipboard access!"));

		return false;
	}

	if (!::EmptyClipboard())
	{
		status.AsyncCreateMessage(AsyncStatusWindow::MSG_ERROR, AsyncStatusWindow::ITEM_GENERAL, destination.general.name,
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
        status.AsyncCreateMessage(AsyncStatusWindow::MSG_CHECK, AsyncStatusWindow::ITEM_GENERAL, destination.general.name,
          _T("Copied image to clipboard"));
        usedClipboard = true;
			}
			catch (const Win32Exception& excp)
			{
				status.AsyncCreateMessage(AsyncStatusWindow::MSG_ERROR, AsyncStatusWindow::ITEM_GENERAL, destination.general.name,
					LibCC::Format(TEXT("Clipboard: Can't copy image: %")).s(excp.What()).Str());
			}
		}
		else
		{
			status.AsyncCreateMessage(AsyncStatusWindow::MSG_ERROR, AsyncStatusWindow::ITEM_GENERAL, destination.general.name,
				TEXT("Clipboard: Can't get clipboard-friendly image data!"));
		}
	}
	else
	{
		status.AsyncCreateMessage(AsyncStatusWindow::MSG_ERROR, AsyncStatusWindow::ITEM_GENERAL, destination.general.name,
			LibCC::Format(TEXT("File: can't get screenshot data!")).s(destination.general.path).Str());
	}

	::CloseClipboard();

	return true;
}

bool ProcessEmailDestination(HWND hwnd, AsyncStatusWindow& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image, bool& usedClipboard)
{
	return true;
}

bool ProcessDestination(HWND hwnd, AsyncStatusWindow& status,
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