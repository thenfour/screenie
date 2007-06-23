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

#include "DestProperties.hpp"
#include "DestinationProperties.hpp"

#include "codec.hpp"
#include "libcc/registry.h"


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

void CDestinationDlg::SetEnabledButtons()
{
	BOOL enableButtons = m_listView.GetSelectedCount() > 0 ? TRUE : FALSE;
	m_editButton.EnableWindow(enableButtons);
	m_removeButton.EnableWindow(enableButtons);
	m_duplicateButton.EnableWindow(enableButtons);

	bool checked = (BST_CHECKED == IsDlgButtonChecked(IDC_ENABLEARCHIVE));
	GetDlgItem(IDC_LIMITARCHIVESTATIC1).EnableWindow(checked);
	GetDlgItem(IDC_LIMITARCHIVESTATIC2).EnableWindow(checked);
	GetDlgItem(IDC_ARCHIVELIMIT).EnableWindow(checked);
}

void CDestinationDlg::PopulateDestinationList()
{
	m_listView.DeleteAllItems();

	for (size_t i = 0; i < m_optionsCopy.GetNumDestinations(); ++i)
	{
		ScreenshotDestination destination;
		if (m_optionsCopy.GetDestination(destination, i))
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

  // auto size those columns
  m_listView.SetColumnWidth(0, LVSCW_AUTOSIZE);
  m_listView.SetColumnWidth(1, LVSCW_AUTOSIZE);

  SetEnabledButtons();
}

LRESULT CDestinationDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// center the dialog on the screen
	CenterWindow();

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

  // Load window placement settings.
  if(m_optionsCopy.HaveConfigPlacement())
  {
    SetWindowPlacement(&m_optionsCopy.GetConfigPlacement());
  }

	m_editButton = GetDlgItem(IDC_EDIT);
	m_removeButton = GetDlgItem(IDC_REMOVE);
	m_duplicateButton = GetDlgItem(IDC_DUPLICATE);

	// set up the list view columns
	m_listView = GetDlgItem(IDC_DESTINATIONS);

	m_listView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

	m_listView.AddColumn(TEXT("Name"), 0);
	m_listView.SetColumnWidth(0, 200);

	m_listView.AddColumn(TEXT("Type"), 1);
	m_listView.SetColumnWidth(1, 100);

	// put the destinations on the list
	PopulateDestinationList();

  // set that OK button text.
  SetDlgItemText(IDOK, m_OKbuttonText.c_str());

	// if necessary, check the checkboxes
	if (m_optionsCopy.IncludeCursor())
		CheckDlgButton(IDC_INCLUDECURSOR, BST_CHECKED);
	if (m_optionsCopy.ShowCropWindow())
		CheckDlgButton(IDC_CROPPING, BST_CHECKED);
	if (m_optionsCopy.ConfirmOptions())
		CheckDlgButton(IDC_CONFIRM, BST_CHECKED);
	if (m_optionsCopy.ShowStatus())
		CheckDlgButton(IDC_SHOWSTATUS, BST_CHECKED);
  if (m_optionsCopy.AutoStartup())
		CheckDlgButton(IDC_AUTOSTART, BST_CHECKED);
  if (m_optionsCopy.ShowSplash())
    CheckDlgButton(IDC_SHOWSPLASH, BST_CHECKED);
	if (m_optionsCopy.EnableArchive())
    CheckDlgButton(IDC_ENABLEARCHIVE, BST_CHECKED);

	SetDlgItemInt(IDC_ARCHIVELIMIT, m_optionsCopy.ArchiveLimit() / 1024 / 1024);

	SetEnabledButtons();

  SetForegroundWindow(*this);

	return TRUE;
}

LRESULT CDestinationDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CloseDialog(false);

	return 0;
}

LRESULT CDestinationDlg::OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	NMLISTVIEW* nmlv = reinterpret_cast<NMLISTVIEW*>(pnmh);

	if (nmlv->uChanged & LVIF_STATE)
	{
		BOOL checked = m_listView.GetCheckState(nmlv->iItem);
		m_optionsCopy.GetDestination(static_cast<size_t>(nmlv->lParam)).enabled = (checked == TRUE);
	}

	SetEnabledButtons();

	return 0;
}

LRESULT CDestinationDlg::OnItemActivated(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
  return OnEditDestination(0,0,0,bHandled);
}

LRESULT CDestinationDlg::OnNewDestination(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ScreenshotDestination destination;

	destination.enabled = true;

	destination.general.name = TEXT("My New Destination");
	destination.general.type = ScreenshotDestination::TYPE_FILE;
	destination.general.imageFormat = TEXT("image/png");
	destination.general.filenameFormat = TEXT("%Y%m%d-%h%i%s.png");

	destination.image.scaleType = ScreenshotDestination::SCALE_NONE;
	destination.image.scalePercent = 100;
	destination.image.maxDimension = 320;

	destination.ftp.hostname = TEXT("ftp.mysite.com");
	destination.ftp.port = 21;
	destination.ftp.username = TEXT("username");
	destination.ftp.SetPassword(TEXT("password"));
	destination.ftp.passwordOptions = ScreenshotDestination::Ftp::PO_Plaintext;
	destination.ftp.remotePath = TEXT("/home/username/public_html/");
	destination.ftp.resultURL = TEXT("http://mysite.com/~username/");
	destination.ftp.copyURL = false;

	GetSpecialFolderPath(destination.general.path, CSIDL_MYPICTURES);

	CDestinationProperties properties(destination, m_optionsCopy, TEXT("Create New Destination"));
	if (properties.DoModal(m_hWnd) == IDOK)
	{
		destination = properties.GetDestination();
		m_optionsCopy.AddDestination(destination);

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
		if (m_optionsCopy.GetDestination(destination, selectedIndex))
		{
			CDestinationProperties prop(destination, m_optionsCopy, TEXT("Edit Destination"));
			if (prop.DoModal(m_hWnd) == IDOK)
			{
				m_optionsCopy.SetDestination(prop.GetDestination(), selectedIndex);

				// one or more destination has changed. repopulate the
				// destination list; the info on the list is probably outdated.

				PopulateDestinationList();
			}
		}
	}

	return 0;
}

LRESULT CDestinationDlg::OnDuplicateDestination(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	int selectedIndex = -1;
	if ((selectedIndex = GetSelectedDestination()) != -1)
	{
		ScreenshotDestination existing;
		if (m_optionsCopy.GetDestination(existing, selectedIndex))
		{
			ScreenshotDestination duplicate = existing;
			duplicate.general.id.CreateNew();
			duplicate.general.name = L"Copy of " + duplicate.general.name;
			m_optionsCopy.AddDestination(duplicate);
			PopulateDestinationList();
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
		if(IDYES == MessageBox(LibCC::Format("Are you sure you want to remove destination %").qs(m_optionsCopy.GetDestination(selectedIndex).general.name).CStr(),
			_T("Delete destination"), MB_YESNO | MB_ICONQUESTION))
		{
			m_optionsCopy.RemoveDestination(selectedIndex);
			// update the destination listview
			PopulateDestinationList();
		}
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
  case IDC_AUTOSTART:
    m_optionsCopy.AutoStartup(checked);
		break;
  case IDC_SHOWSPLASH:
    m_optionsCopy.ShowSplash(checked);
		break;
	case IDC_SHOWSTATUS:
 		m_optionsCopy.ShowStatus(checked);
		break;
	case IDC_CONFIRM:
		m_optionsCopy.ConfirmOptions(checked);
		break;
	case IDC_CROPPING:
		m_optionsCopy.ShowCropWindow(checked);
		break;
	case IDC_INCLUDECURSOR:
		m_optionsCopy.IncludeCursor(checked);
		break;
	case IDC_ENABLEARCHIVE:
		m_optionsCopy.EnableArchive(checked);
		SetEnabledButtons();
		break;
	}

	return 0;
}

LRESULT CDestinationDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_optionsCopy.ArchiveLimit(1024 * 1024 * GetDlgItemInt(IDC_ARCHIVELIMIT));
	CloseDialog(true);

	return 0;
}

LRESULT CDestinationDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CloseDialog(false);

	return 0;
}

void CDestinationDlg::CloseDialog(bool bSaveOptions)
{
  // save window placement
  WINDOWPLACEMENT wp;
  GetWindowPlacement(&wp);
  m_optionsCopy.SetConfigPlacement(wp);

  if(bSaveOptions)
  {
    m_optionsFinal = m_optionsCopy;
		m_optionsFinal.SaveSettings();
  }

  EndDialog(bSaveOptions ? TRUE : FALSE);
}
