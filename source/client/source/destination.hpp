//
// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
//
//
//

#ifndef SCREENIE_XDESTINATION_HPP
#define SCREENIE_XDESTINATION_HPP

// for StatusWindow
#include "StatusDlg.hpp"

// for ScreenshotDestination
#include "ScreenshotDestination.hpp"

bool GetTransformedScreenshot(const ScreenshotDestination::Image& options,
							  std::shared_ptr<Gdiplus::Bitmap> screenshot, std::shared_ptr<Gdiplus::Bitmap>& transformed);

struct DestinationArgs
{
	HWND hwnd;
	CStatusDlg& statusDlg;
	ScreenshotDestination dest;
	std::shared_ptr<Gdiplus::Bitmap> image;
	bool bUsedClipboard;
	ScreenshotID screenshotID;
	ScreenshotNamingData namingData;

	DestinationArgs(HWND wnd, CStatusDlg& dlg, std::shared_ptr<Gdiplus::Bitmap> pImage,
		tstd::tstring title, bool usedClipboard, ScreenshotID id)
		: hwnd(wnd), statusDlg(dlg), image(pImage), bUsedClipboard(usedClipboard), screenshotID(id)
	{
		namingData.windowTitle = title;
	}
};

bool ProcessDestination(DestinationArgs& args);

bool ProcessFtpDestination(DestinationArgs& args);
bool ProcessImageShackDestination(DestinationArgs& args);
bool ProcessFileDestination(DestinationArgs& args);
bool ProcessClipboardDestination(DestinationArgs& args);

#endif