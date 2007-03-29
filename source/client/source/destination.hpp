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

bool ProcessDestination(HWND hwnd, AsyncStatusWindow& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image, const tstd::tstring& windowTitle,  bool& usedClipboard);

bool ProcessFtpDestination(HWND hwnd, AsyncStatusWindow& status, ScreenshotDestination& destination,
	util::shared_ptr<Gdiplus::Bitmap> image, const tstd::tstring& windowTitle, bool& usedClipboard);
bool ProcessFileDestination(HWND hwnd, AsyncStatusWindow& status, ScreenshotDestination& destination,
	util::shared_ptr<Gdiplus::Bitmap> image, const tstd::tstring& windowTitle, bool& usedClipboard);
bool ProcessClipboardDestination(HWND hwnd, AsyncStatusWindow& status, ScreenshotDestination& destination,
	util::shared_ptr<Gdiplus::Bitmap> image, const tstd::tstring& windowTitle, bool& usedClipboard);
bool ProcessEmailDestination(HWND hwnd, AsyncStatusWindow& status, ScreenshotDestination& destination,
	util::shared_ptr<Gdiplus::Bitmap> image, const tstd::tstring& windowTitle, bool& usedClipboard);
bool ProcessScreenieDestination(HWND hwnd, AsyncStatusWindow& status, ScreenshotDestination& destination,
	util::shared_ptr<Gdiplus::Bitmap> image, const tstd::tstring& windowTitle, bool& usedClipboard);

#endif