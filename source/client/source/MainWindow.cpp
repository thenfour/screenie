//
// MainDlg.h - declarations for the destination dialog box
// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
//

#include "stdafx.hpp"
#include "resource.h"

#include "AboutDlg.hpp"
#include "CropDlg.hpp"
#include "ImageEditWnd.hpp"
#include "DestinationDlg.hpp"
#include "MainWindow.hpp"
#include "StatusDlg.hpp"
#include "TextPromptDlg.h"

#include "codec.hpp"
#include "image.hpp"
#include "utility.hpp"
#include "destination.hpp"
#include "clipboard.hpp"

LONG g_GUIEntrancyRefs;

const UINT TIMER_ID_TIMEDSCREENSHOT = 0x333;

std::set<CCropDlg*> CCropDlg::g_instances;

BOOL CMainWindow::DisplayTrayMenu()
{
	CMenu contextMenu(AtlLoadMenu(IDM_CONTEXTMENU));
	CMenuHandle trayMenu = contextMenu.GetSubMenu(0);

	::SetForegroundWindow(m_hWnd);

	POINT cursorPos = { 0 };
	::GetCursorPos(&cursorPos);

	trayMenu.TrackPopupMenu(TPM_RIGHTALIGN, cursorPos.x, cursorPos.y, m_hWnd);

	SendMessage(WM_NULL);
	return TRUE;
}


BOOL CMainWindow::TakeScreenshot(const POINT& cursorPos, BOOL altDown)
{
	CBitmap screenshotBitmap;
	if (GetScreenshotBitmap(screenshotBitmap.m_hBitmap, altDown, m_screenshotOptions.IncludeCursor()))
	{
		std::shared_ptr<Gdiplus::Bitmap> screenshot(new Gdiplus::Bitmap(screenshotBitmap, NULL));
		return ProcessScreenshot(screenshot, true, false, altDown != 0);
	}
	return FALSE;
}

BOOL CMainWindow::ProcessScreenshot(Gdiplus::BitmapPtr screenshot, bool registerInArchive, bool forceCropDlg, bool altDown)
{
	RECT windowRect = { 0 };
	HWND hWndForeground = ::GetForegroundWindow();

	if(forceCropDlg || (m_screenshotOptions.GetScreenshotAction() == SA_SHOWCROP))
	{
		if (!altDown || (altDown && m_screenshotOptions.ShowCropWithAlt()))
		{
			// show cropping dialog
			CCropDlg cropDialog(screenshot, m_screenshotOptions);

			if (cropDialog.DoModal(0) == IDOK)
			{
				// if there is a selection, make that our screenshot
				std::shared_ptr<Gdiplus::Bitmap> croppedScreenshot;
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
	}
	else if(m_screenshotOptions.GetScreenshotAction() == SA_SHOWDESTINATIONS)
	{
		CDestinationDlg dialog(m_screenshotOptions, L"Finish");
		if(TRUE != dialog.DoModal())
			return FALSE;
	}

	if (m_screenshotOptions.ShowStatus())
  {
    ShowStatusWindow();
  }

	// create a thumbnail for the history window
	std::shared_ptr<Gdiplus::Bitmap> thumbnail;
	ResizeBitmap(thumbnail, *screenshot, 100);
	ScreenshotID screenshotID = m_statusDialog.RegisterScreenshot(screenshot, thumbnail);

  unsigned threadID;
  ThreadParams params;

  params.options = m_screenshotOptions;// creates a copy for thread safety
  params.pThis = this;
  params.screenshot = screenshot;
	params.screenshotID = screenshotID;
	params.windowTitle = GetWindowString(hWndForeground);

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

	return TRUE;
}

unsigned __stdcall CMainWindow::ProcessDestinationsThreadProc(void* p)
{
  ThreadParams& params(*((ThreadParams*)p));
  ScreenshotOptions& options(params.options);
  CMainWindow* pThis(params.pThis);

  int enabledDestinations = 0;
  DestinationArgs args(pThis->m_hWnd, pThis->m_statusDialog, params.screenshot, params.windowTitle, false, params.screenshotID);

  for (size_t i = 0; i < options.GetNumDestinations(); ++i)
  {
	  if (options.GetDestination(args.dest, i))
	  {
		  if (args.dest.enabled)
		  {
			  ProcessDestination(args);
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

LRESULT CMainWindow::OnTimer(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	if (wParam == TIMER_ID_TIMEDSCREENSHOT)
	{
		KillTimer(TIMER_ID_TIMEDSCREENSHOT);
		PostMessage(WM_TAKESCREENSHOT, MAKELONG(0, 0), 0);
	}

	return 0;
}

LRESULT CMainWindow::OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
  CreateTrayIcon();

	if (m_statusDialog.Create(0, NULL))
  {
		m_statusDialog.ShowWindow(SW_HIDE);
  }

	RegisterHotKey(*this, 1000, 0, VK_SNAPSHOT);
	RegisterHotKey(*this, 1001, MOD_ALT, VK_SNAPSHOT);
	RegisterHotKey(*this, 1002, MOD_ALT | MOD_CONTROL, VK_SNAPSHOT);// for international keyboards
	return 0;
}

LRESULT CMainWindow::OnHotKey(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	handled = TRUE;
	BOOL altDown = FALSE;
	switch(wParam)
	{
	case 1000:
		break;
	case 1001:
	case 1002:
		altDown = TRUE;
		break;
	default:
		return 0;
	}

	POINT cursorPos = { 0 };
	::GetCursorPos(&cursorPos);

	// simulate default windows screenshot processing. this lets you take screenshots of screenie itself for example.
	{
		CBitmap screenshotBitmap;
		if (GetScreenshotBitmap(screenshotBitmap.m_hBitmap, altDown, m_screenshotOptions.IncludeCursor()))
		{
			Clipboard c(*this);
			//c.SetBitmap(screenshotBitmap.m_hBitmap);
			Gdiplus::Bitmap* bmpTemp = Gdiplus::Bitmap::FromHBITMAP(screenshotBitmap.m_hBitmap, 0);
			c.SetBitmap(bmpTemp);
			delete bmpTemp;
		}
	}

	PostMessage(CMainWindow::WM_TAKESCREENSHOT, MAKELONG(cursorPos.x, cursorPos.y), altDown);

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

void CMainWindow::ShowStatusWindow()
{
	m_statusDialog.ShowWindow(SW_SHOW);
  // for some reason it doesnt like to redraw when it's shown... even non-client area.
  // so, a little brute force and we're golden... like the shower.
  m_statusDialog.SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
  m_statusDialog.SetWindowPos(HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
}

LRESULT CMainWindow::OnTrayTakeScreenshot(WORD notifyCode, WORD id, HWND hWndCtl, BOOL& handled)
{
	// 300ms after the user clicks on the menu item it should be safe to take a screen
	SetTimer(TIMER_ID_TIMEDSCREENSHOT, 250);

	return 0;
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

LRESULT CMainWindow::OnTraySetTimer(WORD notifyCode, WORD id, HWND hWndCtl, BOOL& handled)
{
	CTextPromptDlg dlg(TEXT("Set screenshot timer"), TEXT("Set the number of seconds until the screen capture is triggered"), L"5");

	if (dlg.DoModal() == IDOK)
	{
		int nSeconds = _wtoi(dlg.GetText().c_str());
		
		// 300ms after the user clicks on the menu item it should be safe to take a screen
		SetTimer(TIMER_ID_TIMEDSCREENSHOT, nSeconds * 1000);
	}

	return 0;

}

LRESULT CMainWindow::OnReprocessScreenshot(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
  ScopedGUIEntry ee;
  if(ee.Enter())
  {
		Gdiplus::BitmapPtr screenshot = m_archive.RetrieveImage((ScreenshotID)lParam);
		if(!screenshot)
		{
			// no reason to check options for archive enabled, because there are other runtime reasons for the archive
			// to be disabled, like if there were errors in opening the database originally.
			MessageBox(L"Unfortunately, Screenie is unable to reprocess this screenshot. The most likely cause is that archiving may be disabled.", L"Screenie", MB_OK | MB_ICONEXCLAMATION);
		}
		else
		{
			ProcessScreenshot(screenshot, false, true, false);
		}
  }
	else
	{
		MessageBox(L"Screenie is already busy processing. Try again later.", L"Screenie", MB_OK | MB_ICONEXCLAMATION);
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
	CAboutDlg dlg(this->m_screenshotOptions, this->m_archive);
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
