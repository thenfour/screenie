//
//
//
//
//

#ifndef SCREENIE_XDESTINATION_HPP
#define SCREENIE_XDESTINATION_HPP

// for StatusWindow
#include "StatusDlg.hpp"

// for ScreenshotDestination
#include "ScreenshotDestination.hpp"

bool ProcessDestination(HWND hwnd, IActivity& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image, const tstd::tstring& windowTitle,  bool& usedClipboard, ScreenshotID screenshotID);

bool ProcessFtpDestination(HWND hwnd, IActivity& status, ScreenshotDestination& destination,
	util::shared_ptr<Gdiplus::Bitmap> image, const tstd::tstring& windowTitle, bool& usedClipboard, ScreenshotID screenshotID);
bool ProcessImageShackDestination(HWND hwnd, IActivity& status, ScreenshotDestination& destination,
	util::shared_ptr<Gdiplus::Bitmap> image, const tstd::tstring& windowTitle, bool& usedClipboard, ScreenshotID screenshotID);
bool ProcessFileDestination(HWND hwnd, IActivity& status, ScreenshotDestination& destination,
	util::shared_ptr<Gdiplus::Bitmap> image, const tstd::tstring& windowTitle, bool& usedClipboard, ScreenshotID screenshotID);
bool ProcessClipboardDestination(HWND hwnd, IActivity& status, ScreenshotDestination& destination,
	util::shared_ptr<Gdiplus::Bitmap> image, const tstd::tstring& windowTitle, bool& usedClipboard, ScreenshotID screenshotID);

#endif