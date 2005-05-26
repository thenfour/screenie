//
// DestinationDlg.cpp - implementation of screenie's main options window
// Copyright (c) 2005 Roger Clark
// Copyright (c) 2003 Carl Corcoran
//

#include "stdafx.hpp"
#include "resource.h"

#include "AboutDlg.hpp"
#include "DestinationDlg.hpp"
#include "StatusDlg.hpp"
#include "utility.hpp"
#include "CroppingWnd.hpp"

#include "DestProperties.hpp"
#include "DestinationProperties.hpp"

#include "codec.hpp"

BOOL CDestinationDlg::PreTranslateMessage(MSG* pMsg)
{
	return CWindow::IsDialogMessage(pMsg);
}

void CDestinationDlg::DisplayListContextMenu()
{
	CMenu contextMenu(AtlLoadMenu(IDM_CONTEXTMENU));
	CMenuHandle listMenu = contextMenu.GetSubMenu(1);

	POINT cursorPos = { 0 };
	::GetCursorPos(&cursorPos);

	contextMenu.TrackPopupMenu(TPM_LEFTALIGN, cursorPos.x, cursorPos.y, m_hWnd);
}

int CDestinationDlg::GetSelectedDestination()
{
	if (m_listView.GetSelectedCount())
	{
		LVITEM lvItem = { 0 };

		lvItem.mask = LVIF_PARAM;

		if (m_listView.GetSelectedItem(&lvItem))
		{
			return static_cast<int>(lvItem.lParam);
		}
	}

	return -1;
}

void CDestinationDlg::PopulateDestinationList()
{
	m_listView.DeleteAllItems();

	for (size_t i = 0; i < m_screenshotOptions.GetNumDestinations(); ++i)
	{
		ScreenshotDestination destination;
		if (m_screenshotOptions.GetDestination(destination, i))
		{
			LVITEM item = { 0 };

			// by assigning the item an item index greater than the current number of
			// items, InsertItem will tell us the real index when it returns. we can
			// use that index to set the item properties for the second column.

			item.iItem = (m_listView.GetItemCount() + 1);
			item.mask = LVIF_TEXT | LVIF_PARAM;
			item.lParam = static_cast<LPARAM>(i);

			// const_cast is okay here. the listview isn't going to modify the
			// buffer we send -- since we're setting the item text, not getting it.
			item.pszText = const_cast<LPTSTR>(destination.general.name.c_str());
			item.cchTextMax = destination.general.name.length();

			// now, let's set the info for the second column (the type description)
			int itemIndex = m_listView.InsertItem(&item);

			m_listView.SetItem(itemIndex, 1, LVIF_TEXT,
				ScreenshotDestination::TypeToString(destination.general.type).c_str(), 0, 0, 0, 0);
			m_listView.SetCheckState(itemIndex, destination.enabled);
		}
	}
}

LRESULT CDestinationDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

	// set icons
	HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SCREENIE), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hIcon, TRUE);
	HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_SCREENIE), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hIconSmall, FALSE);

	DlgResize_Init(true, true, WS_CLIPCHILDREN);

	// set up the list view columns
	m_listView = GetDlgItem(IDC_DESTINATIONS);

	m_listView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

	m_listView.AddColumn(TEXT("Name"), 0);
	m_listView.SetColumnWidth(0, 200);

	m_listView.AddColumn(TEXT("Type"), 1);
	m_listView.SetColumnWidth(1, 100);

	// put the destinations on the list
	PopulateDestinationList();

	// if necessary, check the checkboxes
	if (m_screenshotOptions.IncludeCursor())
		CheckDlgButton(IDC_INCLUDECURSOR, BST_CHECKED);
	if (m_screenshotOptions.ShowCropWindow())
		CheckDlgButton(IDC_CROPPING, BST_CHECKED);
	if (m_screenshotOptions.ConfirmOptions())
		CheckDlgButton(IDC_CONFIRM, BST_CHECKED);
	if (m_screenshotOptions.ShowStatus())
		CheckDlgButton(IDC_SHOWSTATUS, BST_CHECKED);

	return TRUE;
}

LRESULT CDestinationDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CloseDialog(IDOK);

	return 0;
}

LRESULT CDestinationDlg::OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	NMLISTVIEW* nmlv = reinterpret_cast<NMLISTVIEW*>(pnmh);

	if (nmlv->uChanged & LVIF_STATE)
	{
		BOOL checked = m_listView.GetCheckState(nmlv->iItem);
		m_screenshotOptions.GetDestination(static_cast<size_t>(nmlv->lParam)).enabled = (checked == TRUE);
	}

	return 0;
}

LRESULT CDestinationDlg::OnNewDestination(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ScreenshotDestination destination;

	destination.enabled = true;

	destination.general.name = TEXT("My New Destination");
	destination.general.type = ScreenshotDestination::TYPE_FILE;
	destination.general.imageFormat = TEXT("image/jpeg");
	destination.general.filenameFormat = TEXT("%Y%m%d-%h%i%s.jpg");

	destination.image.createThumbnail = false;
	destination.image.useFilenameFormat = false;
	destination.image.filenameFormat = TEXT("%Y%m%d-%h%i%s-thumb");
	destination.image.scaleType = ScreenshotDestination::SCALE_LIMITDIMENSIONS;
	destination.image.scalePercent = 100;
	destination.image.maxDimension = 320;
	destination.image.thumbScaleType = ScreenshotDestination::SCALE_LIMITDIMENSIONS;
	destination.image.thumbMaxDimension = 150;
	destination.image.thumbScalePercent = 15;

	destination.ftp.hostname = TEXT("ftp.mysite.com");
	destination.ftp.port = 21;
	destination.ftp.username = TEXT("username");
	destination.ftp.password = TEXT("password");
	destination.ftp.remotePath = TEXT("/home/username/public_html/");
	destination.ftp.resultURL = TEXT("http://mysite.com/~username");
	destination.ftp.copyURL = false;

	GetSpecialFolderPath(destination.general.path, CSIDL_MYPICTURES);

	CDestinationProperties properties(destination, TEXT("Create New Destination"));
	if (properties.DoModal(m_hWnd) == IDOK)
	{
		destination = properties.GetDestination();
		m_screenshotOptions.AddDestination(destination);

		PopulateDestinationList();
	}

	return 0;
}

LRESULT CDestinationDlg::OnEditDestination(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int selectedIndex = -1;
	if ((selectedIndex = GetSelectedDestination()) != -1)
	{
		ScreenshotDestination destination;
		if (m_screenshotOptions.GetDestination(destination, selectedIndex))
		{
			CDestinationProperties prop(destination, TEXT("Edit Destination"));
			if (prop.DoModal(m_hWnd) == IDOK)
			{
				m_screenshotOptions.SetDestination(prop.GetDestination(), selectedIndex);

				// one or more destination has changed. repopulate the
				// destination list; the info on the list is probably outdated.

				PopulateDestinationList();
			}
		}
	}

	return 0;
}

LRESULT CDestinationDlg::OnRemoveDestination(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int selectedIndex = -1;
	if ((selectedIndex = GetSelectedDestination()) != -1)
	{
		// remove the destination from the collection
		m_screenshotOptions.RemoveDestination(selectedIndex);

		// update the destination listview
		PopulateDestinationList();
	}

	return 0;
}

LRESULT CDestinationDlg::OnCheckboxClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	bool checked = false;

	if (::IsDlgButtonChecked(m_hWnd, wID))
		checked = true;

	switch (wID)
	{
		case IDC_SHOWSTATUS:
 			m_screenshotOptions.ShowStatus(checked);
			break;
		case IDC_CONFIRM:
			m_screenshotOptions.ConfirmOptions(checked);
			break;
		case IDC_CROPPING:
			m_screenshotOptions.ShowCropWindow(checked);
			break;
		case IDC_INCLUDECURSOR:
			m_screenshotOptions.IncludeCursor(checked);
			break;
	}

	return 0;
}

LRESULT CDestinationDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseDialog(wID);

	return 0;
}

void CDestinationDlg::CloseDialog(int nVal)
{
	EndDialog(nVal);
}
