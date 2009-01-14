//
// args.dest.cpp - actual processing of FTP destinations
// Copyright (c) 2003 Carl Corcoran
// Copyright (c) 2005 Roger Clark
//

#include "stdafx.hpp"
#include "resource.h"

#include "libcc/winapi.hpp"

// ui
#include "destination.hpp"
#include "StatusDlg.hpp"

// general
#include "clipboard.hpp"
#include "codec.hpp"
#include "image.hpp"
#include "internet.hpp"
#include "path.hpp"
#include "utility.hpp"

#include <curl/curl.h>
#include <curl/easy.h>
#include "curlutil.h"

typedef MSXML2::IXMLDOMDocumentPtr Document;
typedef MSXML2::IXMLDOMElementPtr Element;
typedef MSXML2::IXMLDOMNodePtr Node;

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

bool ProcessImageShackDestination(DestinationArgs& args)
{
	bool success = false;

	EventID msgid = args.statusDlg.RegisterEvent(args.screenshotID, EI_PROGRESS, ET_IMAGESHACK, args.dest.general.name, _T("Initiating ImageShack transfer"));
	args.statusDlg.EventSetProgress(msgid, 0, 1);// set it to 0%

	util::shared_ptr<Gdiplus::Bitmap> transformedImage;
	if (!GetTransformedScreenshot(args.dest.image, args.image, transformedImage))
	{
		args.statusDlg.EventSetText(msgid, L"ImageShack: Can't resize screenshot");
		args.statusDlg.EventSetIcon(msgid, EI_ERROR);
		return false;
	}

	// before we can upload the image, we need to save it to a temporary file.
	tstd::tstring temporaryFilename = GetUniqueTemporaryFilename();
	if (!SaveImageToFile(*transformedImage, args.dest.general.imageFormat, temporaryFilename, args.dest.general.imageQuality))
	{
		args.statusDlg.EventSetText(msgid, L"ImageShack: Can't save image to temporary file.");
		args.statusDlg.EventSetIcon(msgid, EI_ERROR);
		return false;
	}

	ScreenieHttpRequest request(&args.statusDlg, msgid);

	request.AddFile("fileupload", LibCC::ToUTF8(temporaryFilename), LibCC::ToUTF8(args.dest.general.imageFormat));
	request.AddPostArgument("xml", "yes");
	request.AddHeader("Expect: ");

	request.SetURL("http://www.imageshack.us/upload_api.php");

	if (request.Perform())
	{
		std::wstring xml = LibCC::ToUTF16(request.GetData());

		::CoInitialize(NULL);
		Document doc;
		if(SUCCEEDED(doc.CreateInstance(L"Msxml2.DOMDocument")))
		{
			if(VARIANT_TRUE == doc->loadXML(xml.c_str()))
			{
				Element root = doc->selectSingleNode(L"imginfo");
				if(root != 0)
				{
					if (Element links = root->selectSingleNode(L"links"))
					{
						if (Element imagelink = links->selectSingleNode(L"image_link"))
						{
							BSTR bstrURL = { 0 };
							imagelink->get_text(&bstrURL);

							args.statusDlg.EventSetText(msgid, TEXT("Upload complete."));
							args.statusDlg.EventSetIcon(msgid, EI_CHECK);

							args.statusDlg.EventSetText(msgid, LibCC::Format("Uploaded to: %").s(bstrURL).Str());
							args.statusDlg.EventSetURL(msgid, bstrURL);

							if (args.dest.imageshack.copyURL)
							{
								if (args.bUsedClipboard)
								{
									args.statusDlg.RegisterEvent(args.screenshotID, EI_WARNING, ET_GENERAL, args.dest.general.name, _T("Warning: Overwriting clipboard contents"));
								}

								LibCC::Result r = Clipboard(args.hwnd).SetText(bstrURL);
								if(r.Succeeded())
								{
									args.statusDlg.RegisterEvent(args.screenshotID, EI_INFO, ET_GENERAL, args.dest.general.name,
										LibCC::Format("Copied URL to clipboard %").qs(bstrURL).Str(), bstrURL);
									args.bUsedClipboard = true;
								}
								else
								{
									args.statusDlg.RegisterEvent(args.screenshotID, EI_ERROR, ET_GENERAL, args.dest.general.name,
										LibCC::Format(TEXT("Can't copy text to clipboard: %")).s(r.str()).Str(), bstrURL);
								}
							}

							::SysFreeString(bstrURL);
						}
						else
						{
							args.statusDlg.EventSetText(msgid, L"ImageShack: Malformed server response");
							args.statusDlg.EventSetIcon(msgid, EI_ERROR);
						}
					}
				}
				else
				{
					args.statusDlg.EventSetText(msgid, L"ImageShack: Malformed server response");
					args.statusDlg.EventSetIcon(msgid, EI_ERROR);
				}
			}
		}
	}
	else
	{
		// delete the temp file
		args.statusDlg.EventSetText(msgid, LibCC::ToUTF16(request.GetErrorText()));
		args.statusDlg.EventSetIcon(msgid, EI_ERROR);
	}

	// delete the temp file
	DeleteFile(temporaryFilename.c_str());

	return true;
}

bool ProcessFileDestination(DestinationArgs& args)
{
	bool success = false;

	// let's see if the directory they want us to save to even exists.
	if (!::PathFileExists(args.dest.general.path.c_str()))
	{
		args.statusDlg.RegisterEvent(args.screenshotID, EI_ERROR, ET_GENERAL, args.dest.general.name,
			LibCC::Format(TEXT("File: folder \"%\" doesn't exist")).s(args.dest.general.path).Str());
		return false;
	}

	// try to get the transformed screenshot.
	util::shared_ptr<Gdiplus::Bitmap> transformedScreenshot;
	if (!GetTransformedScreenshot(args.dest.image, args.image, transformedScreenshot))
	{
		args.statusDlg.RegisterEvent(args.screenshotID, EI_ERROR, ET_GENERAL, args.dest.general.name,
			LibCC::Format(TEXT("File: can't get screenshot data!")).s(args.dest.general.path).Str());
		return false;
	}

	// let's get the filename and format it.

	SYSTEMTIME systemTime = args.localTime;
	args.dest.GetNowBasedOnTimeSettings(systemTime);

	tstd::tstring filename = FormatFilename(systemTime, args.dest.general.filenameFormat, args.windowTitle);
	tstd::tstring fullPath = LibCC::Format(TEXT("%\\%")).s(args.dest.general.path).s(filename).Str();

	// do the deed
	if (SaveImageToFile(*transformedScreenshot, args.dest.general.imageFormat, fullPath, args.dest.general.imageQuality))
	{
		// get the file size.
		Win32Handle f = CreateFile(fullPath.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0); 
		if(LibCC::IsValidHandle(f.val))
		{
			args.statusDlg.RegisterEvent(args.screenshotID, EI_CHECK, ET_FILE, args.dest.general.name,
				LibCC::Format(L"File (% bytes) saved to %").ul(GetFileSize(f.val, 0)).qs(fullPath).Str(), fullPath);
		}
		else
		{
			args.statusDlg.RegisterEvent(args.screenshotID, EI_CHECK, ET_FILE, args.dest.general.name,
				LibCC::Format(TEXT("File: saved image to %")).qs(fullPath).Str(), fullPath);
		}
	}
	else
	{
    args.statusDlg.RegisterEvent(args.screenshotID, EI_ERROR, ET_GENERAL, args.dest.general.name,
			TEXT("File: couldn't save image to disk"));

		return false;
	}

	return success;
}

bool ProcessClipboardDestination(DestinationArgs& args)
{
  if(args.bUsedClipboard)
  {
		args.statusDlg.RegisterEvent(args.screenshotID, EI_WARNING, ET_GENERAL, args.dest.general.name, _T("Warning: Overwriting clipboard contents"));
  }
  LibCC::Result r;

	// try to get the transformed screenshot.
	util::shared_ptr<Gdiplus::Bitmap> transformedScreenshot;
	if(!GetTransformedScreenshot(args.dest.image, args.image, transformedScreenshot))
  {
		r.Fail(TEXT("File: error getting screenshot data"));
  }
  else
	{
		r = Clipboard(args.hwnd).SetBitmap(transformedScreenshot.get());
		if(r.Succeeded())
		{
			args.statusDlg.RegisterEvent(args.screenshotID, EI_CHECK, ET_GENERAL, args.dest.general.name,
				_T("Copied image to clipboard"));
			args.bUsedClipboard = true;
			r.Succeed();
		}

		//HBITMAP clipboardBitmap = NULL;
		//if (Gdiplus::Ok != transformedScreenshot->GetHBITMAP(Gdiplus::Color(0,0,0), &clipboardBitmap))
  //  {
		//	r.Fail(TEXT("Clipboard: Can't get clipboard-friendly image data"));
  //  }
  //  else
		//{
		//	HBITMAP bitmapCopy = DuplicateScreenshotBitmap(clipboardBitmap);
		//	DeleteObject(clipboardBitmap);

  //    LibCC::Result r = Clipboard(hwnd).SetBitmap(bitmapCopy);
		//	DeleteObject(bitmapCopy);
  //    if(r.Succeeded())
  //    {
		//		args.statusDlg.RegisterEvent(args.screenshotID, EI_CHECK, ET_GENERAL, args.dest.general.name,
  //        _T("Copied image to clipboard"));
  //      usedClipboard = true;
  //      r.Succeed();
		//	}
		//}
	}

  if(!r)
  {
		args.statusDlg.RegisterEvent(args.screenshotID, EI_ERROR, ET_GENERAL, args.dest.general.name, r.str());
  }

	return r.Succeeded();
}


bool ProcessDestination(DestinationArgs& args)
{
	switch (args.dest.general.type)
	{
		case ScreenshotDestination::TYPE_FILE:
			return ProcessFileDestination(args);
			break;
		case ScreenshotDestination::TYPE_FTP:
			return ProcessFtpDestination(args);
			break;
		case ScreenshotDestination::TYPE_IMAGESHACK:
			return ProcessImageShackDestination(args);
			break;
		case ScreenshotDestination::TYPE_CLIPBOARD:
			return ProcessClipboardDestination(args);
			break;
	}

	return false;
}