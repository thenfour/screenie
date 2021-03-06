//
// destination.cpp - actual processing of FTP destinations
// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
//

#include "stdafx.hpp"
#include "resource.h"

#include "libcc/winapi.hpp"
#include "../libgrumble/snarl.h"

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

#include "GrumbleSupport.h"
#include "curlutil.h"
#include "BitlyURL.h"

bool ProcessFtpDestination(DestinationArgs& args)
{
	EventID msgid = args.statusDlg.RegisterEvent(args.screenshotID, EI_PROGRESS, ET_FTP, args.dest.general.name, _T("Initiating FTP transfer"));
	args.statusDlg.EventSetProgress(msgid, 0, 1);// set it to 0%

	std::shared_ptr<Gdiplus::Bitmap> transformedImage;
	if (!GetTransformedScreenshot(args.dest.image, args.image, transformedImage))
	{
		args.statusDlg.EventSetText(msgid, L"FTP: Can't resize screenshot");
		args.statusDlg.EventSetIcon(msgid, EI_ERROR);
		return false;
	}

	// before we can upload the image, we need to save it to a temporary file.
	tstd::tstring temporaryFilename = GetUniqueTemporaryFilename();
	if (!SaveImageToFile(*transformedImage, args.dest.general.imageFormat, temporaryFilename, args.dest.general.imageQuality))
	{
		args.statusDlg.EventSetText(msgid, L"FTP: Can't save image to temporary file.");
		args.statusDlg.EventSetIcon(msgid, EI_ERROR);
		return false;
	}

	// format the destination filename based on the current time
	tstd::tstring remotePath = FormatFilename(args.namingData, args.dest.general.localTime, args.dest.ftp.remotePathFormat);
	tstd::tstring resultingURL = FormatFilename(args.namingData, args.dest.general.localTime, args.dest.ftp.resultURLFormat);

	LibCC::Result r;
	DWORD size = 0;

	ScreenieFtpRequest request(&args.statusDlg, msgid);

	bool needsSlash = (remotePath.size() > 0) && (remotePath.find_first_of(L"\\/") != 0);

	tstd::tstring url = LibCC::Format("ftp://%:%/%%")// there is an extra slash because it's an absolute path. http://curl.haxx.se/mail/archive-2007-10/0053.html
		.s(args.dest.ftp.hostname)
		.ul(args.dest.ftp.port)
		.s(needsSlash ? L"/" : L"")
		.s(remotePath)
		.Str();

	request.SetFilename(LibCC::ToUTF8(temporaryFilename));
	request.SetURL(LibCC::ToUTF8(url));
	request.SetUsername(LibCC::ToUTF8(args.dest.ftp.username));
	request.SetPassword(LibCC::ToUTF8(args.dest.ftp.DecryptPassword()));

	if (!request.Perform())
	{
		args.statusDlg.EventSetIcon(msgid, EI_ERROR);
		args.statusDlg.EventSetText(msgid, LibCC::ToUTF16(request.GetErrorText()));

		return false;
	}

	// delete the temp file
	DeleteFile(temporaryFilename.c_str());

	args.statusDlg.EventSetText(msgid, TEXT("Upload complete."));
	args.statusDlg.EventSetIcon(msgid, EI_CHECK);

	if (!resultingURL.empty())
	{
		//tstd::tstring url = LibCC::Format(TEXT("%%")).s(args.dest.ftp.resultURL).s(remoteFileName).Str();

		Grumble.ShowMessage(L"Uploaded Screenshot", LibCC::Format(TEXT("Successfully uploaded image to:\r\n%")).s(resultingURL).CStr(), 10, L"", 0, L"FTP Upload Complete");

		args.statusDlg.EventSetText(msgid, LibCC::Format("Uploaded % (% KB/s) to %").s(BytesToString(request.GetUploadSize())).d(request.GetUploadSpeed(), 3).s(resultingURL).Str());
		args.statusDlg.EventSetURL(msgid, resultingURL);

		if (args.dest.ftp.copyURL)
		{
			tstd::tstring strFinalURL = resultingURL;
			if (args.dest.ftp.shortenURL)
			{
				BitlyShortenInfo info;
				if (BitlyShortenURL(args, strFinalURL, info))
				{
					strFinalURL = info.shortURL;
				}
			}

			if(args.bUsedClipboard)
			{
				args.statusDlg.RegisterEvent(args.screenshotID, EI_WARNING, ET_GENERAL, args.dest.general.name, _T("Warning: Overwriting clipboard contents"));
			}

			LibCC::Result r = Clipboard(args.hwnd).SetText(strFinalURL);
			if(r.Succeeded())
			{
				args.statusDlg.RegisterEvent(args.screenshotID, EI_INFO, ET_GENERAL, args.dest.general.name,
					LibCC::Format("Copied URL to clipboard %").qs(strFinalURL).Str(), strFinalURL);
				args.bUsedClipboard = true;
			}
			else
			{
				args.statusDlg.RegisterEvent(args.screenshotID, EI_ERROR, ET_GENERAL, args.dest.general.name,
					LibCC::Format(TEXT("Can't copy text to clipboard: %")).s(r.str()).Str(), strFinalURL);
			}
		}
	}
	return true;
}