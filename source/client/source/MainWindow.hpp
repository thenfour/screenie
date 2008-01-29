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

#include "ScreenshotArchive.hpp"


// for entrancy issues around print screen handler & the destinations dialog & such:
extern LONG g_GUIEntrancyRefs;


// a hacky sort of critical section
class ScopedGUIEntry
{
public:
  ~ScopedGUIEntry()
  {
    InterlockedDecrement(&g_GUIEntrancyRefs);
  }
  bool Enter() const
  {
    if(InterlockedIncrement(&g_GUIEntrancyRefs) > 1)
    {
      // oops ref count greater than one. get out.
      return false;
    }
    return true;
  }
};


class CMainWindow :
  public CWindowImpl<CMainWindow>
{
public:
	DECLARE_WND_CLASS(TEXT("ScreenieMainWnd"))

	enum {
		WM_NOTIFYICON = WM_APP + 1,
		WM_SHOWSCREENIE = WM_APP + 2,
		WM_TAKESCREENSHOT = WM_APP + 3,
		WM_REPROCESSSCREENSHOT = WM_APP + 4,
	};

  CMainWindow(ScreenshotOptions& options, ScreenshotArchive& archive) :
    m_screenshotOptions(options),
		m_archive(archive),
    m_statusDialog(options, archive)
  {
    m_uTaskbarCreatedMsg = RegisterWindowMessage(_T("TaskbarCreated"));
    m_iconData.hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SCREENIE), 
		  IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
  }
  ~CMainWindow()
  {
    DestroyIcon(m_iconData.hIcon);
  }

	BEGIN_MSG_MAP(CMainWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_HOTKEY, OnHotKey)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_NOTIFYICON, OnNotifyIcon)
		MESSAGE_HANDLER(WM_TAKESCREENSHOT, OnTakeScreenshot)
		MESSAGE_HANDLER(WM_REPROCESSSCREENSHOT, OnReprocessScreenshot)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(m_uTaskbarCreatedMsg, OnTaskbarCreated)

		COMMAND_ID_HANDLER(ID_SHOWSTATUSWINDOW, OnShowStatusWindow)
		COMMAND_ID_HANDLER(ID_TRAYCONTEXTMENU_ABOUT, OnAbout)
		COMMAND_ID_HANDLER(ID_TRAYCONTEXTMENU_CONFIGURE, OnConfigure)
		COMMAND_ID_HANDLER(ID_TRAYCONTEXTMENU_EXIT, OnExit)
	END_MSG_MAP()

	BOOL DisplayTrayMenu();

	BOOL TakeScreenshot(const POINT& cursorPos, BOOL altDown);
	BOOL ProcessScreenshot(Gdiplus::BitmapPtr screenshot, bool registerInArchive, bool forceCropDlg);

	//
	// message handlers
	//

	LRESULT OnTaskbarCreated(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnNotifyIcon(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnTakeScreenshot(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnReprocessScreenshot(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnHotKey(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled);
	LRESULT OnClose(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
  {
    handled = TRUE;
	  DestroyWindow();
    return 0;
  }

	LRESULT OnBuy(WORD notifyCode, WORD id, HWND hWndCtl, BOOL& handled);
	LRESULT OnAbout(WORD notifyCode, WORD id, HWND hWndCtl, BOOL& handled);
	LRESULT OnConfigure(WORD notifyCode, WORD id, HWND hWndCtl, BOOL& handled);
	LRESULT OnExit(WORD notifyCode, WORD id, HWND hWndCtl, BOOL& handled);
  LRESULT OnShowStatusWindow(WORD notifyCode, WORD id, HWND hWndCtl, BOOL& handled);
  void ShowStatusWindow();

  // returns true if settings are used ("OK" button was clicked).
  // returns false if canceled.
  bool OnConfigure(const tstd::tstring& OKbuttonText);

	int CreateStatusMessage();

private:

  struct ThreadParams
  {
    CMainWindow* pThis;
    ScreenshotOptions options;
    util::shared_ptr<Gdiplus::Bitmap> screenshot;
		tstd::tstring windowTitle;
		ScreenshotID screenshotID;
  };

  static unsigned __stdcall ProcessDestinationsThreadProc(void*);


  UINT m_uTaskbarCreatedMsg;

  bool CreateTrayIcon();
  NOTIFYICONDATA m_iconData;

  CStatusDlg m_statusDialog;
	ScreenshotOptions& m_screenshotOptions;
	ScreenshotArchive& m_archive;
};

#endif