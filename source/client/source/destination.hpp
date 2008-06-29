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

bool GetTransformedScreenshot(const ScreenshotDestination::Image& options,
							  util::shared_ptr<Gdiplus::Bitmap> screenshot, util::shared_ptr<Gdiplus::Bitmap>& transformed);

struct DestinationArgs
{
	HWND hwnd;
	CStatusDlg& statusDlg;
	ScreenshotDestination dest;
	util::shared_ptr<Gdiplus::Bitmap> image;
	tstd::tstring windowTitle;
	bool bUsedClipboard;
	ScreenshotID screenshotID;
	SYSTEMTIME localTime;

	DestinationArgs(HWND wnd, CStatusDlg& dlg, util::shared_ptr<Gdiplus::Bitmap> pImage,
		tstd::tstring title, bool usedClipboard, ScreenshotID id)
		: hwnd(wnd), statusDlg(dlg), image(pImage), windowTitle(title), bUsedClipboard(usedClipboard), screenshotID(id)
	{
	}
};

bool ProcessDestination(DestinationArgs& args);

bool ProcessFtpDestination(DestinationArgs& args);
bool ProcessImageShackDestination(DestinationArgs& args);
bool ProcessFileDestination(DestinationArgs& args);
bool ProcessClipboardDestination(DestinationArgs& args);

#endif