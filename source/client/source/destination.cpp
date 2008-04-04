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

bool ProcessImageShackDestination(HWND hwnd, IActivity& status, ScreenshotDestination& destination,
	util::shared_ptr<Gdiplus::Bitmap> image, const tstd::tstring& windowTitle, bool& usedClipboard, ScreenshotID screenshotID)
{
	bool success = false;

	EventID msgid = status.RegisterEvent(screenshotID, EI_PROGRESS, ET_IMAGESHACK, destination.general.name, _T("Initiating ImageShack transfer"));
	status.EventSetProgress(msgid, 0, 1);// set it to 0%

	util::shared_ptr<Gdiplus::Bitmap> transformedImage;
	if (!GetTransformedScreenshot(destination.image, image, transformedImage))
	{
		status.EventSetText(msgid, L"ImageShack: Can't resize screenshot");
		status.EventSetIcon(msgid, EI_ERROR);
		return false;
	}

	// before we can upload the image, we need to save it to a temporary file.
	tstd::tstring temporaryFilename = GetUniqueTemporaryFilename();
	if (!SaveImageToFile(*transformedImage, destination.general.imageFormat, temporaryFilename, destination.general.imageQuality))
	{
		status.EventSetText(msgid, L"ImageShack: Can't save image to temporary file.");
		status.EventSetIcon(msgid, EI_ERROR);
		return false;
	}

	SYSTEMTIME systemTime = { 0 };
	destination.GetNowBasedOnTimeSettings(systemTime);
	tstd::tstring remoteFileName = FormatFilename(systemTime, destination.general.filenameFormat, windowTitle);

	ScreenieHttpRequest request(&status, msgid);

	request.AddFile("fileupload", LibCC::ToUTF8(temporaryFilename), LibCC::ToUTF8(destination.general.imageFormat), LibCC::ToUTF8(remoteFileName));
	request.AddPostArgument("xml", "yes");
	request.AddHeader("Expect: ");

	request.SetURL("http://www.imageshack.us/upload_api.php");

	if (request.Perform())
	{
		std::wstring xml = LibCC::ToUnicode(request.GetData());

		::CoInitialize(NULL);
		Document doc;
		if(SUCCEEDED(doc.CreateInstance(L"Msxml2.DOMDocument")))
		{
			if(VARIANT_TRUE == doc->loadXML(xml.c_str()))
			{
				Element root = doc->selectSingleNode(L"links");
				if(root != 0)
				{
					if (Element imagelink = root->selectSingleNode(L"image_link"))
					{
						BSTR bstrURL = { 0 };
						imagelink->get_text(&bstrURL);

						status.EventSetText(msgid, TEXT("Upload complete."));
						status.EventSetIcon(msgid, EI_CHECK);

						status.EventSetText(msgid, LibCC::Format("Uploaded to: %").s(bstrURL).Str());
						status.EventSetURL(msgid, bstrURL);

						if (destination.imageshack.copyURL)
						{
							if (usedClipboard)
							{
								status.RegisterEvent(screenshotID, EI_WARNING, ET_GENERAL, destination.general.name, _T("Warning: Overwriting clipboard contents"));
							}

							LibCC::Result r = Clipboard(hwnd).SetText(bstrURL);
							if(r.Succeeded())
							{
								status.RegisterEvent(screenshotID, EI_INFO, ET_GENERAL, destination.general.name,
									LibCC::Format("Copied URL to clipboard %").qs(bstrURL).Str(), bstrURL);
								usedClipboard = true;
							}
							else
							{
								status.RegisterEvent(screenshotID, EI_ERROR, ET_GENERAL, destination.general.name,
									LibCC::Format(TEXT("Can't copy text to clipboard: %")).s(r.str()).Str(), bstrURL);
							}
						}

						::SysFreeString(bstrURL);
					}
					else
					{
						status.EventSetText(msgid, L"ImageShack: Malformed server response");
						status.EventSetIcon(msgid, EI_ERROR);
					}
				}
				else
				{
					status.EventSetText(msgid, L"ImageShack: Malformed server response");
					status.EventSetIcon(msgid, EI_ERROR);
				}
			}
		}
	}
	else
	{
		// delete the temp file
		status.EventSetText(msgid, LibCC::ToUnicode(request.GetErrorText()));
		status.EventSetIcon(msgid, EI_ERROR);
	}

	// delete the temp file
	DeleteFile(temporaryFilename.c_str());

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
	if (SaveImageToFile(*transformedScreenshot, destination.general.imageFormat, fullPath, destination.general.imageQuality))
	{
		// get the file size.
		Win32Handle f = CreateFile(fullPath.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0); 
		if(LibCC::IsValidHandle(f.val))
		{
			status.RegisterEvent(screenshotID, EI_CHECK, ET_FILE, destination.general.name,
				LibCC::Format(L"File (% bytes) saved to %").ul(GetFileSize(f.val, 0)).qs(fullPath).Str(), fullPath);
		}
		else
		{
			status.RegisterEvent(screenshotID, EI_CHECK, ET_FILE, destination.general.name,
				LibCC::Format(TEXT("File: saved image to %")).qs(fullPath).Str(), fullPath);
		}
	}
	else
	{
    status.RegisterEvent(screenshotID, EI_ERROR, ET_GENERAL, destination.general.name,
			TEXT("File: couldn't save image to disk"));

		return false;
	}

	return success;
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
		r = Clipboard(hwnd).SetBitmap(transformedScreenshot.get());
		if(r.Succeeded())
		{
			status.RegisterEvent(screenshotID, EI_CHECK, ET_GENERAL, destination.general.name,
				_T("Copied image to clipboard"));
			usedClipboard = true;
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
		//		status.RegisterEvent(screenshotID, EI_CHECK, ET_GENERAL, destination.general.name,
  //        _T("Copied image to clipboard"));
  //      usedClipboard = true;
  //      r.Succeed();
		//	}
		//}
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
		case ScreenshotDestination::TYPE_IMAGESHACK:
			return ProcessImageShackDestination(hwnd, status, destination, image, windowTitle, usedClipboard, screenshotID);
			break;
		case ScreenshotDestination::TYPE_CLIPBOARD:
			return ProcessClipboardDestination(hwnd, status, destination, image, windowTitle, usedClipboard, screenshotID);
			break;
	}

	return false;
}