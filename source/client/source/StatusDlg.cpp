//
//
//
//
//

#include "stdafx.hpp"
#include "resource.h"

#include "clipboard.hpp"
#include "StatusDlg.hpp"

BOOL CStatusDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

BOOL CStatusDlg::OnIdle()
{
	return FALSE;
}

//
// StatusWindow implementation
//

void CStatusDlg::ClearMessages()
{
	if (m_listView.IsWindow())
		m_listView.DeleteAllItems();
}

void CStatusDlg::PrintMessage(const MessageType type, const tstd::tstring& destination,
	const tstd::tstring& message)
{
	if (m_listView.IsWindow())
	{
		int itemID = m_listView.GetItemCount() + 1;

		itemID = m_listView.AddItem(itemID, 0, destination.c_str(), (type == MSG_ERROR) ? 1 : 0);
		m_listView.SetItem(itemID, 1, LVIF_TEXT, message.c_str(), 0, 0, 0, 0);
	}
}

//

LRESULT CStatusDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow();
	DlgResize_Init(true, true, WS_CLIPCHILDREN);

  // Load window placement settings.
  if(m_options.HaveStatusPlacement())
  {
    SetWindowPlacement(&m_options.GetStatusPlacement());
  }

	// create and populate image list for message icons
	if (m_imageList.Create(16, 16, ILC_COLOR32, 2, 0))
	{
		m_imageList.AddIcon(::LoadIcon(NULL, IDI_INFORMATION));
		m_imageList.AddIcon(::LoadIcon(NULL, IDI_ERROR));

		// set up the list view for messages
		m_listView = GetDlgItem(IDC_MESSAGES);

		if (m_listView.IsWindow())
		{
			m_listView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
			m_listView.SetImageList(m_imageList, LVSIL_SMALL);

			m_listView.AddColumn(TEXT("Destination"), 0);
			m_listView.SetColumnWidth(0, 75);

			m_listView.AddColumn(TEXT("Message"), 1);
			m_listView.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
		}
	}

	return TRUE;
}

LRESULT CStatusDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CloseDialog(IDOK);

	return 0;
}

LRESULT CStatusDlg::OnChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

	return 0;
}

LRESULT CStatusDlg::OnRightClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	OnCopy(0, 0, 0, bHandled);

	NMITEMACTIVATE* itemActivate = reinterpret_cast<NMITEMACTIVATE*>(pnmh);

	CMenu contextMenu(AtlLoadMenu(IDM_CONTEXTMENU));
	CMenuHandle listMenu = contextMenu.GetSubMenu(2);

	POINT cursorPos = { 0 };
	::GetCursorPos(&cursorPos);

	listMenu.TrackPopupMenu(TPM_LEFTALIGN, cursorPos.x, cursorPos.y, m_hWnd);

	return 0;
}

LRESULT CStatusDlg::OnCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	LVITEM item = { 0 };

	TCHAR textBuffer[1024] = { 0 };
	item.cchTextMax = 1024;
	item.pszText = textBuffer;
	item.mask = LVIF_TEXT;
	item.iSubItem = 1;

	if (m_listView.GetSelectedItem(&item))
	{
		try
		{
			Clipboard(m_hWnd).SetText(item.pszText);
		}
		catch (const Win32Exception& excp)
		{
#ifdef _DEBUG
			MessageBox(excp.What().c_str(), TEXT("Clipboard Error"),
				MB_OK | MB_ICONERROR);
#endif
		}
	}

	return 0;
}

LRESULT CStatusDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseDialog(IDOK);

	return 0;
}

void CStatusDlg::CloseDialog(int nVal)
{
	// the next time the user sees this dialog, he probably doesn't want to
	// see messages pertaining to previously processed screenshots
	ClearMessages();

  // save window placement
  WINDOWPLACEMENT wp;
  GetWindowPlacement(&wp);
  m_options.SetStatusPlacement(wp);

	// this doesn't actually close the dialog box.
	ShowWindow(SW_HIDE);
}
