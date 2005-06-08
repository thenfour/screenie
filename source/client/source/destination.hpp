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

bool ProcessDestination(HWND hwnd, StatusWindow& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image, bool& usedClipboard);

bool ProcessFtpDestination(HWND hwnd, StatusWindow& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image, bool& usedClipboard);
bool ProcessFileDestination(HWND hwnd, StatusWindow& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image, bool& usedClipboard);
bool ProcessClipboardDestination(HWND hwnd, StatusWindow& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image, bool& usedClipboard);
bool ProcessEmailDestination(HWND hwnd, StatusWindow& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image, bool& usedClipboard);

#endif