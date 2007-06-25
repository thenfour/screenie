

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
  }
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
  {
		m_listView.DeleteAllItems();
  }
}

LRESULT CStatusDlg::OnDeleteAllItems(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
  bHandled = TRUE;
  // tell the listview to call OnDeleteItem() for every single item in it.
  return FALSE;
}

LRESULT CStatusDlg::OnDeleteItem(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
  bHandled = TRUE;
  NMLISTVIEW& nmlv(*((NMLISTVIEW*)pnmh));
  ItemSpec* p = reinterpret_cast<ItemSpec*>(m_listView.GetItemData(nmlv.iItem));
  delete p;
  return 0;
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

	// create and populate image list for message icons
	if (m_imageList.Create(16, 16, ILC_COLOR32, 2, 0))
	{
		m_iconInfo = m_imageList.AddIcon(::LoadIcon(NULL, IDI_INFORMATION));
    m_iconWarning = m_imageList.AddIcon(::LoadIcon(NULL, IDI_WARNING));
		m_iconError = m_imageList.AddIcon(::LoadIcon(NULL, IDI_ERROR));

    HICON hCheck = (HICON)::LoadImage(_Module.GetResourceInstance(),
			MAKEINTRESOURCE(IDI_CHECK), IMAGE_ICON, 16, 16, 0);
    m_iconCheck = m_imageList.AddIcon(hCheck);
    DestroyIcon(hCheck);

    // set up the list view for messages
		m_listView = GetDlgItem(IDC_MESSAGES);

		m_activity.Attach(GetDlgItem(IDC_ACTIVITY));

    m_progress.InitializeProgressImages(m_imageList,
      COLORREFToRgbPixel32(m_listView.GetBkColor()),
      MakeRgbPixel32(222,123,16),
      MakeRgbPixel32(0,40,86));

      //COLORREFToRgbPixel32(GetSysColor(COLOR_ACTIVECAPTION)),
      //COLORREFToRgbPixel32(GetSysColor(COLOR_GRADIENTACTIVECAPTION)) );

		if (m_listView.IsWindow())
		{
			m_listView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
			m_listView.SetImageList(m_imageList, LVSIL_SMALL);

			m_listView.AddColumn(TEXT("Destination"), 0);
			m_listView.SetColumnWidth(0, LVSCW_AUTOSIZE);

			m_listView.AddColumn(TEXT("Message"), 1);
			m_listView.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
		}
	}

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

LRESULT CStatusDlg::OnChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

	return 0;
}

LRESULT CStatusDlg::OnRightClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
  NMITEMACTIVATE* itemActivate = reinterpret_cast<NMITEMACTIVATE*>(pnmh);
  ItemSpec* spec = 0;
  if(itemActivate->iItem != -1)
  {
    spec = ItemToItemSpec(itemActivate->iItem);
  }

	CMenu menu;
  menu.CreatePopupMenu();
  int pos = 0;

  if(spec)
  {
    switch(spec->type)
    {
    default:
    case ET_GENERAL:
      break;
    case ET_FTP:
      menu.InsertMenuItem(pos ++, TRUE, MenuItemInfo::CreateText(_T("Copy URL"), ID_COPYURL));
      menu.InsertMenuItem(pos ++, TRUE, MenuItemInfo::CreateText(_T("Open URL..."), ID_OPENURL));
      break;
    case ET_FILE:
      menu.InsertMenuItem(pos ++, TRUE, MenuItemInfo::CreateText(_T("Copy path"), ID_COPYURL));
      menu.InsertMenuItem(pos ++, TRUE, MenuItemInfo::CreateText(_T("Explore..."), ID_EXPLORE));
      menu.InsertMenuItem(pos ++, TRUE, MenuItemInfo::CreateText(_T("Open file..."), ID_OPENFILE));
      break;
    }
  }  
  // append global items
  if(pos > 0)
  {
    menu.InsertMenuItem(pos ++, TRUE, MenuItemInfo::CreateSeparator());
  }
  menu.InsertMenuItem(pos ++, TRUE, MenuItemInfo::CreateText(_T("Copy message text"), ID_COPYMESSAGE));
  menu.InsertMenuItem(pos ++, TRUE, MenuItemInfo::CreateText(_T("Clear all"), ID_CLEAR));

	POINT cursorPos = { 0 };
	::GetCursorPos(&cursorPos);
	menu.TrackPopupMenu(0, cursorPos.x, cursorPos.y, m_hWnd);

	return 0;
}

LRESULT CStatusDlg::OnClear(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  ClearMessages();
  return 0;
}

LRESULT CStatusDlg::OnExplore(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  ItemSpec* p = GetSelectedItemSpec();
  if(!p)
  {
    return 0;
  }
  tstd::tstring cmdLine = LibCC::Format("explorer /select, %").qs(p->url).Str();

  PROCESS_INFORMATION pi;
  STARTUPINFO si;
  GetStartupInfo(&si);
  LibCC::Blob<TCHAR> stupidBullshit(cmdLine.size() + 1);
  _tcscpy(stupidBullshit.GetBuffer(), cmdLine.c_str());
  if(CreateProcess(0, stupidBullshit.GetBuffer(), 0, 0, FALSE, 0, 0, 0, &si, &pi))
  {
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
  }

  return 0;
}

LRESULT CStatusDlg::OnOpenFile(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  ItemSpec* p = GetSelectedItemSpec();
  if(!p)
  {
    return 0;
  }
  ShellExecute(m_hWnd, _T("open"), p->url.c_str(), NULL, NULL, SW_SHOW);
  return 0;
}

LRESULT CStatusDlg::OnOpenURL(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  ItemSpec* p = GetSelectedItemSpec();
  if(!p)
  {
    return 0;
  }
  ShellExecute(m_hWnd, _T("open"), p->url.c_str(), NULL, NULL, SW_SHOW);
  return 0;
}

LRESULT CStatusDlg::OnCopyURL(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
  ItemSpec* p = GetSelectedItemSpec();
  if(!p)
  {
    return 0;
  }

  LibCC::Result r = Clipboard(m_hWnd).SetText(p->url);
#ifdef _DEBUG
  if(!r)
  {
    MessageBox(r.str().c_str(), TEXT("Clipboard Error"), MB_OK | MB_ICONERROR);
  }
#endif
	return 0;
}

LRESULT CStatusDlg::OnCopyMessage(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	LVITEM item = { 0 };

	TCHAR textBuffer[1024] = { 0 };
	item.cchTextMax = 1024;
	item.pszText = textBuffer;
	item.mask = LVIF_TEXT;
	item.iSubItem = 1;

	if (m_listView.GetSelectedItem(&item))
	{
		LibCC::Result r = Clipboard(m_hWnd).SetText(item.pszText);
#ifdef _DEBUG
    if(!r)
    {
      MessageBox(r.str().c_str(), TEXT("Clipboard Error"), MB_OK | MB_ICONERROR);
    }
#endif
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

ScreenshotID CStatusDlg::RegisterScreenshot(Gdiplus::BitmapPtr image)
{
//	ScreenshotID x = m_activity.RegisterScreenshot(image);
	//return x;
	return 0;
}

EventID CStatusDlg::RegisterEvent(ScreenshotID screenshotID, EventIcon icon, EventType type, const tstd::tstring& destination, const tstd::tstring& message, const tstd::tstring& url)
{
  EventID ret = 0;

	if (m_listView.IsWindow())
	{
		int itemID = m_listView.GetItemCount() + 1;
    ItemSpec* newSpec = new ItemSpec;
    ret = reinterpret_cast<EventID>(newSpec);

		newSpec->archiveCookie = 0;
		newSpec->archiveCookie = m_archive.RegisterEvent(screenshotID, icon, type, destination, message, url);

    newSpec->type = type;
    newSpec->url = url;

		itemID = m_listView.AddItem(itemID, 0, destination.c_str(), EventIconToIconIndex(icon));
		m_listView.SetItemData(itemID, (LPARAM)ret);
		m_listView.SetItemText(itemID, 1, message.c_str());

    m_listView.SetColumnWidth(0, LVSCW_AUTOSIZE);
    m_listView.SetColumnWidth(1, LVSCW_AUTOSIZE);
	}

  return ret;
}

void CStatusDlg::EventSetIcon(EventID msgID, EventIcon icon)
{
  CriticalSection::ScopeLock lock(m_cs);
  int item;
  if(-1 != (item = EventIDToItemID(msgID)))
  {
    m_listView.SetItem(item, 0, LVIF_IMAGE, 0, EventIconToIconIndex(icon), 0, 0, 0);
		m_archive.EventSetIcon(msgID, icon);
  }
}

void CStatusDlg::EventSetProgress(EventID msgID, int pos, int total)
{
  int item;
  if(-1 != (item = EventIDToItemID(msgID)))
  {
    int iimage = m_progress.GetImageFromProgress(pos, total);
    if(pos >= total)
    {
      // 100% - use a special image.
      iimage = EventIconToIconIndex(EI_CHECK);
    }
    m_listView.SetItem(item, 0, LVIF_IMAGE, 0, iimage, 0, 0, 0);
  }
}

void CStatusDlg::EventSetText(EventID msgID, const tstd::tstring& msg)
{
  int item;
  if(-1 != (item = EventIDToItemID(msgID)))
  {
    m_listView.SetItemText(item, 1, msg.c_str());
		m_archive.EventSetText(msgID, msg);
  }
}

void CStatusDlg::EventSetURL(EventID msgID, const tstd::tstring& url)
{
  ItemSpec* pItem = EventIDToItemSpec(msgID);
  if(pItem)
  {
    pItem->url = url;
		m_archive.EventSetURL(msgID, url);
  }
}

int CStatusDlg::EventIDToItemID(EventID msgID)
{
  LVFINDINFO fi = {0};
  fi.flags = LVFI_PARAM;
  fi.lParam = (LPARAM)msgID;
  return m_listView.FindItem(&fi, -1);
}

CStatusDlg::ItemSpec* CStatusDlg::ItemToItemSpec(int id)
{
  if(id < m_listView.GetItemCount() && id >= 0)
  {
    return reinterpret_cast<ItemSpec*>(m_listView.GetItemData(id));
  }
  return 0;
}

CStatusDlg::ItemSpec* CStatusDlg::EventIDToItemSpec(EventID msgID)
{
  if(-1 == EventIDToItemID(msgID))
  {
    return 0;
  }
  return reinterpret_cast<ItemSpec*>(msgID);
}

CStatusDlg::ItemSpec* CStatusDlg::GetSelectedItemSpec()
{
  return ItemToItemSpec(m_listView.GetSelectedIndex());
}
