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

#include <curl/curl.h>

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
  IActivity* status;
  EventID msgid;
};

bool ProcessFtpDestination_ProgressProc(DWORD completed, DWORD total, void* pUser)
{
  ProcessFtpDestination_Info& info(*((ProcessFtpDestination_Info*)pUser));
	info.status->EventSetProgress(info.msgid, static_cast<int>(completed), static_cast<int>(total));
  return true;
}

bool ProcessFtpDestination(HWND hwnd, IActivity& status, ScreenshotDestination& destination,
						   util::shared_ptr<Gdiplus::Bitmap> image, const tstd::tstring& windowTitle, bool& usedClipboard, ScreenshotID screenshotID)
{
	EventID msgid = status.RegisterEvent(screenshotID, EI_PROGRESS, ET_FTP, destination.general.name, _T("Initiating FTP transfer"));
	status.EventSetProgress(msgid, 0, 1);// set it to 0%

	util::shared_ptr<Gdiplus::Bitmap> transformedImage;
	if (!GetTransformedScreenshot(destination.image, image, transformedImage))
	{
		status.EventSetText(msgid, L"FTP: Can't resize screenshot");
		status.EventSetIcon(msgid, EI_ERROR);
		return false;
	}

	// before we can upload the image, we need to save it to a temporary file.
	tstd::tstring temporaryFilename = GetUniqueTemporaryFilename();
	if (!SaveImageToFile(*transformedImage, destination.general.imageFormat, temporaryFilename))
	{
		status.EventSetText(msgid, L"FTP: Can't save image to temporary file.");
		status.EventSetIcon(msgid, EI_ERROR);
		return false;
	}

	// format the destination filename based on the current time
	SYSTEMTIME systemTime = { 0 };
  destination.GetNowBasedOnTimeSettings(systemTime);
	tstd::tstring remoteFileName = FormatFilename(systemTime, destination.general.filenameFormat, windowTitle);

  LibCC::Result r;
  // set up info struct to pass to the progress proc.
  ProcessFtpDestination_Info info;
  info.msgid = msgid;
  info.status = &status;
  if(!(r = UploadFTPFile(destination, temporaryFilename, remoteFileName, 4000, ProcessFtpDestination_ProgressProc, &info)))
  {
    status.EventSetIcon(msgid, EI_ERROR);
    status.EventSetText(msgid, r.str());
    return false;
  }

  // delete the temp file
  DeleteFile(temporaryFilename.c_str());

  status.EventSetText(msgid, TEXT("Upload complete."));
  status.EventSetIcon(msgid, EI_CHECK);

	if (!destination.ftp.resultURL.empty())
	{
    tstd::tstring url = LibCC::Format(TEXT("%%")).s(destination.ftp.resultURL).s(remoteFileName).Str();
    status.EventSetText(msgid, LibCC::Format("Uploaded to: %").s(url).Str());
		status.EventSetURL(msgid, url);

		if (destination.ftp.copyURL)
		{
      if(usedClipboard)
      {
				status.RegisterEvent(screenshotID, EI_WARNING, ET_GENERAL, destination.general.name, _T("Warning: Overwriting clipboard contents"));
      }

      LibCC::Result r = Clipboard(hwnd).SetText(url);
      if(r.Succeeded())
      {
				status.RegisterEvent(screenshotID, EI_INFO, ET_GENERAL, destination.general.name,
          LibCC::Format("Copied URL to clipboard %").qs(url).Str());
        usedClipboard = true;
			}
      else
			{
				status.RegisterEvent(screenshotID, EI_ERROR, ET_GENERAL, destination.general.name,
          LibCC::Format(TEXT("Can't copy text to clipboard: %")).s(r.str()).Str());
			}
		}
	}
	return true;
}

bool ProcessFileDestination(HWND hwnd, IActivity& status, ScreenshotDestination& destination,
	util::shared_ptr<Gdiplus::Bitmap> image, const tstd::tstring& windowTitle, bool& usedClipboard, ScreenshotID screenshotID)
{
	bool success = false;

	// let's see if the directory they want us to save to even exists.
	if (!::PathFileExists(destination.general.path.c_str()))
	{
		status.RegisterEvent(screenshotID, EI_ERROR, ET_GENERAL, destination.general.name,
			LibCC::Format(TEXT("File: folder \"%\" doesn't exist")).s(destination.general.path).Str());
		return false;
	}

	// try to get the transformed screenshot.
	util::shared_ptr<Gdiplus::Bitmap> transformedScreenshot;
	if (!GetTransformedScreenshot(destination.image, image, transformedScreenshot))
	{
		status.RegisterEvent(screenshotID, EI_ERROR, ET_GENERAL, destination.general.name,
			LibCC::Format(TEXT("File: can't get screenshot data!")).s(destination.general.path).Str());
		return false;
	}

	// let's get the filename and format it.

	SYSTEMTIME systemTime = { 0 };
  destination.GetNowBasedOnTimeSettings(systemTime);

	tstd::tstring filename = FormatFilename(systemTime, destination.general.filenameFormat, windowTitle);
	tstd::tstring fullPath = LibCC::Format(TEXT("%\\%")).s(destination.general.path).s(filename).Str();

	// do the deed
	if (SaveImageToFile(*transformedScreenshot, destination.general.imageFormat, fullPath))
	{
    status.RegisterEvent(screenshotID, EI_CHECK, ET_FILE, destination.general.name,
			LibCC::Format(TEXT("File: saved image to %")).qs(fullPath).Str(), fullPath);
	}
	else
	{
    status.RegisterEvent(screenshotID, EI_ERROR, ET_GENERAL, destination.general.name,
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

	::ReleaseDC(NULL, desktopDC);

	return destBitmap;
}

bool ProcessClipboardDestination(HWND hwnd, IActivity& status, ScreenshotDestination& destination,
								 util::shared_ptr<Gdiplus::Bitmap> image, const tstd::tstring& windowTitle, bool& usedClipboard, ScreenshotID screenshotID)
{
  if(usedClipboard)
  {
		status.RegisterEvent(screenshotID, EI_WARNING, ET_GENERAL, destination.general.name, _T("Warning: Overwriting clipboard contents"));
  }
  LibCC::Result r;

	// try to get the transformed screenshot.
	util::shared_ptr<Gdiplus::Bitmap> transformedScreenshot;
	if(!GetTransformedScreenshot(destination.image, image, transformedScreenshot))
  {
		r.Fail(TEXT("File: error getting screenshot data"));
  }
  else
	{
		HBITMAP clipboardBitmap = NULL;
		if (Gdiplus::Ok != transformedScreenshot->GetHBITMAP(Gdiplus::Color(0,0,0), &clipboardBitmap))
    {
			r.Fail(TEXT("Clipboard: Can't get clipboard-friendly image data"));
    }
    else
		{
			HBITMAP bitmapCopy = DuplicateScreenshotBitmap(clipboardBitmap);
			DeleteObject(clipboardBitmap);

      LibCC::Result r = Clipboard(hwnd).SetBitmap(bitmapCopy);
			DeleteObject(bitmapCopy);
      if(r.Succeeded())
      {
				status.RegisterEvent(screenshotID, EI_CHECK, ET_GENERAL, destination.general.name,
          _T("Copied image to clipboard"));
        usedClipboard = true;
        r.Succeed();
			}
		}
	}

  if(!r)
  {
		status.RegisterEvent(screenshotID, EI_ERROR, ET_GENERAL, destination.general.name, r.str());
  }

	return r.Succeeded();
}


bool ProcessDestination(HWND hwnd, IActivity& status, ScreenshotDestination& destination,
	util::shared_ptr<Gdiplus::Bitmap> image, const tstd::tstring& windowTitle, bool& usedClipboard, ScreenshotID screenshotID)
{
	switch (destination.general.type)
	{
		case ScreenshotDestination::TYPE_FILE:
			return ProcessFileDestination(hwnd, status, destination, image, windowTitle, usedClipboard, screenshotID);
			break;
		case ScreenshotDestination::TYPE_FTP:
			return ProcessFtpDestination(hwnd, status, destination, image, windowTitle, usedClipboard, screenshotID);
			break;
		case ScreenshotDestination::TYPE_CLIPBOARD:
			return ProcessClipboardDestination(hwnd, status, destination, image, windowTitle, usedClipboard, screenshotID);
			break;
	}

	return false;
}