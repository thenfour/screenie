//
// MainWindow.hpp - screenie's hidden control window
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#ifndef SCREENIE_MAINWINDOW_HPP
#define SCREENIE_MAINWINDOW_HPP

// for std::auto_ptr
#include <memory>

// for ScreenshotDestination
#include "ScreenshotDestination.hpp"

// for ScreenshotOptions
#include "ScreenshotOptions.hpp"

// for StatusWindow, CStatusDialog
#include "StatusDlg.hpp"

class CMainWindow : public CWindowImpl<CMainWindow>
{
public:
	DECLARE_WND_CLASS(TEXT("ScreenieMainWnd"))

	enum {
		WM_NOTIFYICON = WM_APP + 1,
		WM_SHOWSCREENIE = WM_APP + 2,
		WM_TAKESCREENSHOT = WM_APP + 3
	};

	bool IsProcessing() { return m_processing; }

	BEGIN_MSG_MAP(CMainWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_NOTIFYICON, OnNotifyIcon)
		MESSAGE_HANDLER(WM_TAKESCREENSHOT, OnTakeScreenshot)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)

		COMMAND_ID_HANDLER(ID_TRAYCONTEXTMENU_ABOUT, OnAbout)
		COMMAND_ID_HANDLER(ID_TRAYCONTEXTMENU_CONFIGURE, OnConfigure)
		COMMAND_ID_HANDLER(ID_TRAYCONTEXTMENU_EXIT, OnExit)
	END_MSG_MAP()

	BOOL DisplayTrayMenu();

	BOOL TakeScreenshot(const POINT& cursorPos, BOOL altDown);

	//
	// message handlers
	//

	LRESULT OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnNotifyIcon(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnTakeScreenshot(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);

	LRESULT OnAbout(WORD notifyCode, WORD id, HWND hWndCtl, BOOL& handled);
	LRESULT OnConfigure(WORD notifyCode, WORD id, HWND hWndCtl, BOOL& handled);
	LRESULT OnExit(WORD notifyCode, WORD id, HWND hWndCtl, BOOL& handled);
private:
	bool m_processing;
	CStatusDlg m_statusDialog;
	ScreenshotOptions m_screenshotOptions;
};

#endif