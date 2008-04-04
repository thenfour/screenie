//
// destination.cpp - actual processing of FTP destinations
// Copyright (c) 2003 Carl Corcoran
// Copyright (c) 2005 Roger Clark
//

#include "stdafx.hpp"
#include "resource.h"

#include "libcc/winapi.h"

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

#include "curlutil.h"

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
	if (!SaveImageToFile(*transformedImage, destination.general.imageFormat, temporaryFilename, destination.general.imageQuality))
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
	DWORD size = 0;

	ScreenieFtpRequest request(&status, msgid);

	tstd::tstring url = LibCC::Format("ftp://%:%/%%")
		.s(destination.ftp.hostname)
		.ul(destination.ftp.port)
		.s(destination.ftp.remotePath)
		.s(remoteFileName)
		.Str();

	request.SetFilename(LibCC::ToUTF8(temporaryFilename));
	request.SetURL(LibCC::ToUTF8(url));
	request.SetUsername(LibCC::ToUTF8(destination.ftp.username));
	request.SetPassword(LibCC::ToUTF8(destination.ftp.DecryptPassword()));

	if (!request.Perform())
	{
		status.EventSetIcon(msgid, EI_ERROR);
		status.EventSetText(msgid, LibCC::ToUnicode(request.GetErrorText()));

		return false;
	}

	// delete the temp file
	DeleteFile(temporaryFilename.c_str());

	status.EventSetText(msgid, TEXT("Upload complete."));
	status.EventSetIcon(msgid, EI_CHECK);

	if (!destination.ftp.resultURL.empty())
	{
		tstd::tstring url = LibCC::Format(TEXT("%%")).s(destination.ftp.resultURL).s(remoteFileName).Str();

		status.EventSetText(msgid, LibCC::Format("Uploaded % bytes (% KB/s) to %").ui(request.GetUploadSize()).d(request.GetUploadSpeed(), 3).s(url).Str());
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
					LibCC::Format("Copied URL to clipboard %").qs(url).Str(), url);
				usedClipboard = true;
			}
			else
			{
				status.RegisterEvent(screenshotID, EI_ERROR, ET_GENERAL, destination.general.name,
					LibCC::Format(TEXT("Can't copy text to clipboard: %")).s(r.str()).Str(), url);
			}
		}
	}
	return true;
}