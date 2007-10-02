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

struct ProcessCurlDestination_Info
{
	IActivity* status;
	EventID msgid;
};

int ProcessCurlDestination_ProgressProc(void *ptr, double dltotal, double dlnow, double ultotal, double ulnow)
{
	ProcessCurlDestination_Info& info(*((ProcessCurlDestination_Info*)ptr));
	info.status->EventSetProgress(info.msgid, static_cast<int>(ulnow), static_cast<int>(ultotal));

	return 0;
}

size_t ProcessCurlDestination_WriteProc(void* ptr, size_t size, size_t nmemb, void* userdata)
{
	std::vector<unsigned char>& buffer = *(reinterpret_cast< std::vector<unsigned char>* >(userdata));

	size_t datasize = nmemb * size;
	unsigned char* data = reinterpret_cast<unsigned char*>(ptr);
	buffer.insert(buffer.end(), data, data + datasize);

	return datasize;
}

// bool LoadFileContents(const TCHAR* filename, std::vector<unsigned char>& contents, bool null)
// {
// 	HANDLE hFile = ::CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, NULL);
// 	if (hFile == INVALID_HANDLE_VALUE || hFile == 0)
// 		return false;
// 
// 	bool success = true;
// 
// 	DWORD dwFileSize = 0;
// 	::GetFileSize(hFile, &dwFileSize);
// 
// 	contents.resize(null ? dwFileSize + 1 : dwFileSize, 0);
// 
// 	DWORD dwRead = 0;
// 	if (!ReadFile(hFile, &contents[0], dwFileSize, &dwRead, NULL))
// 		success = false;
// 
// 	::CloseHandle(hFile);
// 
// 	return true;
// }

bool ProcessImageShackDestination(HWND hwnd, IActivity& status, ScreenshotDestination& destination,
	util::shared_ptr<Gdiplus::Bitmap> image, const tstd::tstring& windowTitle, bool& usedClipboard, ScreenshotID screenshotID)
{
	bool success = false;

	EventID msgid = status.RegisterEvent(screenshotID, EI_PROGRESS, ET_IMAGESHACK, destination.general.name, _T("Initiating ImageShack transfer"));
	status.EventSetProgress(msgid, 0, 1);// set it to 0%

	CURL* curl = curl_easy_init();
	if (curl == 0)
	{
		status.EventSetText(msgid, L"ImageShack: Can't initialize libcurl");
		status.EventSetIcon(msgid, EI_ERROR);

		return false;
	}

	util::shared_ptr<Gdiplus::Bitmap> transformedImage;
	if (!GetTransformedScreenshot(destination.image, image, transformedImage))
	{
		status.EventSetText(msgid, L"ImageShack: Can't resize screenshot");
		status.EventSetIcon(msgid, EI_ERROR);
		return false;
	}

	// before we can upload the image, we need to save it to a temporary file.
	tstd::tstring temporaryFilename = GetUniqueTemporaryFilename();
	if (!SaveImageToFile(*transformedImage, destination.general.imageFormat, temporaryFilename))
	{
		status.EventSetText(msgid, L"ImageShack: Can't save image to temporary file.");
		status.EventSetIcon(msgid, EI_ERROR);
		return false;
	}

	std::string temporaryFilenameA = LibCC::ToUTF8(temporaryFilename);

	curl_httppost* post = 0;
	curl_httppost* last = 0;
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "fileupload", CURLFORM_FILE, temporaryFilenameA.c_str(), CURLFORM_END);
	curl_formadd(&post, &last, CURLFORM_COPYNAME, "xml", CURLFORM_COPYCONTENTS, "yes", CURLFORM_END);

	char errorBuffer[CURL_ERROR_SIZE] = { 0 };

	// pretend like we're firefox
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.1) Gecko/20061204 Firefox/2.0.0.1");
	curl_easy_setopt(curl, CURLOPT_URL, "http://www.imageshack.us/index.php");
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);

	ProcessCurlDestination_Info info;
	info.msgid = msgid;
	info.status = &status;

	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, ProcessCurlDestination_ProgressProc);
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, static_cast<void*>(&info));

	std::vector<unsigned char> buffer;
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ProcessCurlDestination_WriteProc);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, static_cast<void*>(&buffer));

	curl_slist* headerlist = 0;
	curl_slist_append(headerlist, "Expect: ");
//	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

	curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

	CURLcode code = curl_easy_perform(curl);
	switch (code)
	{
	case CURLE_OK:
		{
			status.EventSetText(msgid, TEXT("Upload complete."));
			status.EventSetIcon(msgid, EI_CHECK);

			buffer.push_back(0);
			std::wstring xml = LibCC::ToUnicode((const char*)&buffer[0]);

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

			break;
		}
	default:
		// delete the temp file
		status.EventSetText(msgid, LibCC::ToUnicode(errorBuffer));
		status.EventSetIcon(msgid, EI_ERROR);

		break;
	}

	curl_formfree(post);
	curl_slist_free_all(headerlist);

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
		case ScreenshotDestination::TYPE_IMAGESHACK:
			return ProcessImageShackDestination(hwnd, status, destination, image, windowTitle, usedClipboard, screenshotID);
			break;
		case ScreenshotDestination::TYPE_CLIPBOARD:
			return ProcessClipboardDestination(hwnd, status, destination, image, windowTitle, usedClipboard, screenshotID);
			break;
	}

	return false;
}