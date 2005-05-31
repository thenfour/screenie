//
// MainDlg.h - declarations for the destination dialog box
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#include "stdafx.hpp"
#include "resource.h"

#include "AboutDlg.hpp"
#include "CropDlg.hpp"
#include "CroppingWnd.hpp"
#include "DestinationDlg.hpp"
#include "MainWindow.hpp"
#include "StatusDlg.hpp"

#include "codec.hpp"
#include "image.hpp"
#include "utility.hpp"
#include "destination.hpp"

BOOL CMainWindow::DisplayTrayMenu()
{
	CMenu contextMenu(AtlLoadMenu(IDM_CONTEXTMENU));
	CMenuHandle trayMenu = contextMenu.GetSubMenu(0);

	::SetForegroundWindow(m_hWnd);

	POINT cursorPos = { 0 };
	::GetCursorPos(&cursorPos);

	trayMenu.TrackPopupMenu(TPM_LEFTALIGN, cursorPos.x, cursorPos.y, m_hWnd);

	SendMessage(WM_NULL);

	return TRUE;
}

BOOL CMainWindow::TakeScreenshot(const POINT& cursorPos, BOOL altDown)
{
	if (!m_screenshotOptions.GetNumDestinations())
		return FALSE;

	RECT windowRect = { 0 };

	//if (altDown)
	//	::GetWindowRect(::GetForegroundWindow(), &windowRect);
	//else
	//	::GetWindowRect(::GetDesktopWindow(), &windowRect);

	HBITMAP screenshotBitmap = NULL;
	if (GetScreenshotBitmap(screenshotBitmap, altDown, m_screenshotOptions.IncludeCursor()))
	{
		util::shared_ptr<Gdiplus::Bitmap> screenshot(new Gdiplus::Bitmap(screenshotBitmap, NULL));

	  if (m_screenshotOptions.ConfirmOptions())
	  {
		  BOOL handledDummy = FALSE;
      OnConfigure(_T("Next..."));
	  }

    m_processing = true;

		if (m_screenshotOptions.ShowCropWindow())
		{
			// show cropping dialog
			CCropDlg cropDialog(screenshot, m_screenshotOptions);

			if (cropDialog.DoModal(m_hWnd) == IDOK)
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
		}

		if (m_statusDialog.IsWindow())
		{
			if (m_screenshotOptions.ShowStatus())
      {
				m_statusDialog.ShowWindow(SW_SHOW);
        // for some reason it doesnt like to redraw when it's shown... even non-client area.
        // so, a little brute force and we're golden... like the shower.
        m_statusDialog.SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
      }
		}

		for (size_t i = 0; i < m_screenshotOptions.GetNumDestinations(); ++i)
		{
			ScreenshotDestination destination;
			if (m_screenshotOptions.GetDestination(destination, i))
			{
				if (destination.enabled)
					ProcessDestination(m_hWnd, m_statusDialog, destination, screenshot);
			}
		}

		m_processing = false;
	}

	::DeleteObject(screenshotBitmap);

	return TRUE;
}

LRESULT CMainWindow::OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	LoadOptionsFromRegistry(m_screenshotOptions, HKEY_CURRENT_USER, TEXT("Software\\Screenie2"));

	if (m_statusDialog.Create(m_hWnd, NULL))
		m_statusDialog.ShowWindow(SW_HIDE);

	m_processing = false;

	return 0;
}

LRESULT CMainWindow::OnNotifyIcon(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	switch (lParam)
	{
  case WM_LBUTTONDBLCLK:
    OnConfigure(0,0,0,handled);
    break;
  case WM_CONTEXTMENU:
	case WM_RBUTTONUP:
		{
			DisplayTrayMenu();
			break;
		}
	}

	return 0;
}

LRESULT CMainWindow::OnTakeScreenshot(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	POINT cursorPos = { LOWORD(wParam), HIWORD(wParam) };
	BOOL altDown = static_cast<BOOL>(lParam);

	TakeScreenshot(cursorPos, altDown);

	return 0;

}
LRESULT CMainWindow::OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
{
	SaveOptionsToRegistry(m_screenshotOptions, HKEY_CURRENT_USER, TEXT("Software\\Screenie2"));

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
  OnConfigure(_T("Ok"));
	return 0;
}

bool CMainWindow::OnConfigure(const tstd::tstring& OKbuttonText)
{
	CDestinationDlg dialog(m_screenshotOptions, OKbuttonText);
	return (TRUE == dialog.DoModal());
}

LRESULT CMainWindow::OnExit(WORD notifyCode, WORD id, HWND hWndCtl, BOOL& handled)
{
	DestroyWindow();
	PostQuitMessage(0);

	return 0;
}
