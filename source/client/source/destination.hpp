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
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image);

bool ProcessFtpDestination(HWND hwnd, StatusWindow& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image);
bool ProcessFileDestination(HWND hwnd, StatusWindow& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image);
bool ProcessClipboardDestination(HWND hwnd, StatusWindow& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image);
bool ProcessEmailDestination(HWND hwnd, StatusWindow& status,
	ScreenshotDestination& destination, util::shared_ptr<Gdiplus::Bitmap> image);

#endif