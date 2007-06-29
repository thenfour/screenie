

#include "stdafx.hpp"
#include "resource.h"

#include "clipboard.hpp"
#include "StatusDlg.hpp"

BOOL CStatusDlg::PreTranslateMessage(MSG* pMsg)
{
  if(pMsg->message == WM_KEYDOWN)
  {
    if(pMsg->wParam == VK_RETURN)
    {
      CloseDialog(IDOK);
    }
		else if(pMsg->wParam == VK_ESCAPE)
		{
			CloseDialog(IDOK);
		}
  }
  return CWindow::IsDialogMessage(pMsg);
}

BOOL CStatusDlg::OnIdle()
{
	return FALSE;
}

LRESULT CStatusDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow();

  CMessageLoop* pLoop = _Module.GetMessageLoop();
  pLoop->AddMessageFilter(this);

	// set icons
  if(m_hIcon) DestroyIcon(m_hIcon);
  if(m_hIconSmall) DestroyIcon(m_hIconSmall);
	m_hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SCREENIE), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(m_hIcon, TRUE);
	m_hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SCREENIE), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(m_hIconSmall, FALSE);

  
  DlgResize_Init(true, true, WS_CLIPCHILDREN);

	m_activity.Attach(GetDlgItem(IDC_ACTIVITY));

	return TRUE;
}

LRESULT CStatusDlg::OnShowWindow(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	static bool firstShow = true;

	// wParam == TRUE if the window is being shown.
	if (firstShow && wParam)
	{
		if(m_options.HaveStatusPlacement())
		{
			SetWindowPlacement(&m_options.GetStatusPlacement());
		}

		firstShow = false;
	}

	return 0;
}

LRESULT CStatusDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CloseDialog(IDOK);
	return 0;
}

LRESULT CStatusDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseDialog(IDOK);
	return 0;
}

void CStatusDlg::CloseDialog(int nVal)
{
  // save window placement
  WINDOWPLACEMENT wp;
  GetWindowPlacement(&wp);
  m_options.SetStatusPlacement(wp);

	// this doesn't actually close the dialog box.
	ShowWindow(SW_HIDE);
}

LRESULT CStatusDlg::OnDrawItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DRAWITEMSTRUCT& dis = *((DRAWITEMSTRUCT*)lParam);
	if(dis.CtlID == IDC_ACTIVITY)
	{
		return m_activity.OnDrawItem(dis, bHandled);
	}
	bHandled = FALSE;
	return 0;
}

LRESULT CStatusDlg::OnMeasureItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	MEASUREITEMSTRUCT& mis = *((MEASUREITEMSTRUCT*)lParam);
	if(mis.CtlID == IDC_ACTIVITY)
	{
		return m_activity.OnMeasureItem(mis, bHandled);
	}
	bHandled = FALSE;
	return 0;
}

LRESULT CStatusDlg::OnDeleteItem(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DELETEITEMSTRUCT& s = *((DELETEITEMSTRUCT*)lParam);
	if(s.CtlID == IDC_ACTIVITY)
	{
		return m_activity.OnDeleteItem(s, bHandled);
	}
	bHandled = FALSE;
	return 0;
}

ScreenshotID CStatusDlg::RegisterScreenshot(Gdiplus::BitmapPtr image, Gdiplus::BitmapPtr thumbnail)
{
	return m_activity.RegisterScreenshot(image, thumbnail);
}

EventID CStatusDlg::RegisterEvent(ScreenshotID screenshotID, EventIcon icon, EventType type, const tstd::tstring& destination, const tstd::tstring& message, const tstd::tstring& url)
{
	return m_activity.RegisterEvent(screenshotID, icon, type, destination, message, url);
}

void CStatusDlg::EventSetIcon(EventID msgID, EventIcon icon)
{
	m_activity.EventSetIcon(msgID, icon);
}

void CStatusDlg::EventSetProgress(EventID msgID, int pos, int total)
{
	m_activity.EventSetProgress(msgID, pos, total);
}

void CStatusDlg::EventSetText(EventID msgID, const tstd::tstring& msg)
{
	m_activity.EventSetText(msgID, msg);
}

void CStatusDlg::EventSetURL(EventID msgID, const tstd::tstring& url)
{
	m_activity.EventSetURL(msgID, url);
}

void CStatusDlg::DeleteEvent(EventID eventID)
{
	m_activity.DeleteEvent(eventID);
}

void CStatusDlg::DeleteScreenshot(ScreenshotID screenshotID)
{
	m_activity.DeleteScreenshot(screenshotID);
}
