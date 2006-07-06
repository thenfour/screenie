//
// MainDlg.h - declarations for the destination dialog box
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#include "stdafx.hpp"
#include "resource.h"

#include "AboutDlg.hpp"
#include "CropDlg.hpp"
#include "ImageEditWnd.hpp"
#include "DestinationDlg.hpp"
#include "MainWindow.hpp"
#include "StatusDlg.hpp"

#include "codec.hpp"
#include "image.hpp"
#include "utility.hpp"
#include "destination.hpp"

LONG g_GUIEntrancyRefs;


BOOL CMainWindow::DisplayTrayMenu()
{
	CMenu contextMenu(AtlLoadMenu(IDM_CONTEXTMENU));
	CMenuHandle trayMenu = contextMenu.GetSubMenu(0);

#ifdef CRIPPLED
  MenuItemInfo buy = MenuItemInfo::CreateText(_T("Buy Screenie..."), ID_TRAYCONTEXTMENU_BUY);
  MenuItemInfo sep = MenuItemInfo::CreateSeparator();
  trayMenu.InsertMenuItem(0, TRUE, &sep);
  trayMenu.InsertMenuItem(0, TRUE, &buy);
#endif

	::SetForegroundWindow(m_hWnd);

	POINT cursorPos = { 0 };
	::GetCursorPos(&cursorPos);

	trayMenu.TrackPopupMenu(TPM_RIGHTALIGN, cursorPos.x, cursorPos.y, m_hWnd);

	SendMessage(WM_NULL);
	return TRUE;
}

BOOL CMainWindow::TakeScreenshot(const POINT& cursorPos, BOOL altDown)
{
	RECT windowRect = { 0 };

	CBitmap screenshotBitmap;
	if (GetScreenshotBitmap(screenshotBitmap.m_hBitmap, altDown, m_screenshotOptions.IncludeCursor()))
	{
		util::shared_ptr<Gdiplus::Bitmap> screenshot(new Gdiplus::Bitmap(screenshotBitmap, NULL));

	  if (m_screenshotOptions.ConfirmOptions())
	  {
		  BOOL handledDummy = FALSE;
      OnConfigure(_T("Next..."));
	  }

		if (m_screenshotOptions.ShowCropWindow())
		{
			// show cropping dialog
			CCropDlg cropDialog(screenshot, m_screenshotOptions);

			if (cropDialog.DoModal(0) == IDOK)
			{
				// if there is a selection, make that our screenshot

				util::shared_ptr<Gdiplus::Bitmap> croppedScreenshot;
				if (cropDialog.GetCroppedScreenshot(croppedScreenshot))
				{
					// shared_ptr's value semantics take care of the destruction
					// of the previous pointed-to bitmap data
					screenshot = croppedScreenshot;
				}
			}
			else
			{
				return FALSE;
			}
		}

    

		if (m_screenshotOptions.ShowStatus())
    {
      ShowStatusWindow();
    }

    //for(int aoeu = 0; aoeu <= 110; aoeu ++)
    //{
    //  LPARAM id = m_statusDialog.CreateProgressMessage(_T("test"), LibCC::Format("%^%").i(aoeu).Str());
    //  m_statusDialog.MessageSetProgress(id, aoeu, 100);
    //}

    unsigned threadID;
    ThreadParams params;
    params.options = m_screenshotOptions;// creates a copy for thread safety
    params.pThis = this;
    params.screenshot = screenshot;
    HANDLE hThread = (HANDLE)_beginthreadex(0, 0, ProcessDestinationsThreadProc, &params, 0, &threadID);
    while(WAIT_TIMEOUT == WaitForSingleObject(hThread, 0))
    {
      Sleep(1);
      // cycle the message loop.
      MSG msg;
      while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
    CloseHandle(hThread);
  }

	return TRUE;
}

unsigned __stdcall CMainWindow::ProcessDestinationsThreadProc(void* p)
{
  ThreadParams& params(*((ThreadParams*)p));
  ScreenshotOptions& options(params.options);
  CMainWindow* pThis(params.pThis);

  int enabledDestinations = 0;
  bool bUsedClipboard = false;// used to display warnings about copying things multiply to the clipboard
	for (size_t i = 0; i < options.GetNumDestinations(); ++i)
	{
		ScreenshotDestination destination;
		if (options.GetDestination(destination, i))
		{
			if (destination.enabled)
      {
#ifdef CRIPPLED
        if(enabledDestinations == 0)
        {
#endif
          ProcessDestination(pThis->m_hWnd, pThis->m_statusDialog, destination, params.screenshot, bUsedClipboard);
#ifdef CRIPPLED
        }
        else
        {
          // display a warning
          pThis->m_statusDialog.AsyncCreateMessage(
            AsyncStatusWindow::MSG_WARNING, AsyncStatusWindow::ITEM_GENERAL, destination.general.name,
			      _T("This demo only allows you to use one destination per screenshot."));
        }
#endif
        enabledDestinations ++;
      }
		}
	}
  return 0;
}

LRESULT CMainWindow::OnTaskbarCreated(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
  CreateTrayIcon();
  return 0;
}

LRESULT CMainWindow::OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
  CreateTrayIcon();

	if (m_statusDialog.Create(0, NULL))
  {
		m_statusDialog.ShowWindow(SW_HIDE);
  }

	return 0;
}

LRESULT CMainWindow::OnNotifyIcon(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	switch (lParam)
	{
  case WM_LBUTTONDBLCLK:
    {
      OnConfigure(0,0,0,handled);
      break;
    }
  case WM_CONTEXTMENU:
	case WM_RBUTTONUP:
		{
      ScopedGUIEntry ee;
      if(ee.Enter())
      {
    	  DisplayTrayMenu();
      }
			break;
		}
	}
	return 0;
}

LRESULT CMainWindow::OnBuy(WORD notifyCode, WORD id, HWND hWndCtl, BOOL& handled)
{
  // taken from atlctrlx.h / hyperlink control::Navigate()
  SHELLEXECUTEINFO shExeInfo = { sizeof(SHELLEXECUTEINFO), 0, 0, _T("open"), _T("http://screenie.net"), 0, 0, SW_SHOWNORMAL, 0, 0, 0, 0, 0, 0, 0 };
	::ShellExecuteEx(&shExeInfo);
  return 0;
}

void CMainWindow::ShowStatusWindow()
{
	m_statusDialog.ShowWindow(SW_SHOW);
  // for some reason it doesnt like to redraw when it's shown... even non-client area.
  // so, a little brute force and we're golden... like the shower.
  m_statusDialog.SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
  m_statusDialog.SetWindowPos(HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
}

LRESULT CMainWindow::OnShowStatusWindow(WORD notifyCode, WORD id, HWND hWndCtl, BOOL& handled)
{
  ShowStatusWindow();
  handled = TRUE;
  return 0;
}

LRESULT CMainWindow::OnTakeScreenshot(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
  ScopedGUIEntry ee;
  if(ee.Enter())
  {
	  POINT cursorPos = { LOWORD(wParam), HIWORD(wParam) };
	  BOOL altDown = static_cast<BOOL>(lParam);

  	TakeScreenshot(cursorPos, altDown);
  }
	return 0;

}
LRESULT CMainWindow::OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
  if(m_statusDialog.IsWindow())// prevent assertions from wtl
  {
    m_statusDialog.DestroyWindow();
  }

  ::Shell_NotifyIcon(NIM_DELETE, &m_iconData);
	PostQuitMessage(0);
	return 0;
}

LRESULT CMainWindow::OnAbout(WORD notifyCode, WORD id, HWND hWndCtl, BOOL& handled)
{
	CAboutDlg dlg;
	dlg.DoModal();

	return 0;
}

LRESULT CMainWindow::OnConfigure(WORD notifyCode, WORD id, HWND hWndCtl, BOOL& handled)
{
  OnConfigure(_T("OK"));
  return 0;
}

bool CMainWindow::OnConfigure(const tstd::tstring& OKbuttonText)
{
  ScopedGUIEntry ee;
  if(!ee.Enter()) return false;
	CDestinationDlg dialog(m_screenshotOptions, OKbuttonText);
	return (TRUE == dialog.DoModal());
}

LRESULT CMainWindow::OnExit(WORD notifyCode, WORD id, HWND hWndCtl, BOOL& handled)
{
  handled = TRUE;
	DestroyWindow();
	return 0;
}

bool CMainWindow::CreateTrayIcon()
{
  bool r = false;

	m_iconData.cbSize = sizeof(m_iconData);
	m_iconData.hWnd = m_hWnd;
	m_iconData.uID = 0x1BADD00D;
	m_iconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_iconData.uCallbackMessage = CMainWindow::WM_NOTIFYICON;
	m_iconData.uVersion = NOTIFYICON_VERSION;
  _tcscpy(m_iconData.szTip, _T("Screenie Screen Capture Utility"));

	if (::Shell_NotifyIcon(NIM_ADD, &m_iconData))
	{
    r = true;
  }

  return r;
}
