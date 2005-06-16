

#include "stdafx.hpp"
#include "resource.h"
#include "geom.h"
#include "image.hpp"

#include "clipboard.hpp"
#include "StatusDlg.hpp"

void ProgressImages::DrawHLine(long x1, long x2, long y)
{
  // draw horizontal line.
  long xleft = min(x1, x2);
  long xright = max(x1, x2) + 1;
  while(xleft != xright)
  {
    m_bmp->SetPixel(xleft, y, PositionToColor(xleft, y));
    xleft ++;
  }
}

void ProgressImages::DrawAlphaPixel(long cx, long cy, long x, long y, long f, long fmax)
{
  m_bmp->SetPixel(cx+x, cy+y, MixColorsInt(f, fmax, PositionToColor(cx+x, cy+y), m_background));
  m_bmp->SetPixel(cx+x, cy-y-1, MixColorsInt(f, fmax, PositionToColor(cx+x, cy-y-1), m_background));
  m_bmp->SetPixel(cx-x-1, cy+y, MixColorsInt(f, fmax, PositionToColor(cx-x-1, cy+y), m_background));
  m_bmp->SetPixel(cx-x-1, cy-y-1, MixColorsInt(f, fmax, PositionToColor(cx-x-1, cy-y-1), m_background));
}

// x and y are relative to the 
RgbPixel32 ProgressImages::PositionToColor(long x, long y)
{
  if(m_i == m_perimeter) return m_filled;// 100% one is all filled.
  float a = m_angles.GetAngle(x - m_radius, y - m_radius);// GetAngle() needs coords relative to the center of the circle

  // rotate it 90 degrees counter-clockwise.
  a += static_cast<float>(m_perimeter) / 4;
  if(a < 0) a += m_perimeter;
  if(a > m_perimeter) a -= m_perimeter;

  /* to fake anti-aliasing inside the pie, just calculate using a narrow gradient.
   --filled---k====a====l----unfilled------
              |---------| = blur size.
    the gradient is from k to l.  i may be anywhere in this diagram.
  */
  // get the distance from i to the center of the blurring window.
  float aa = a + (m_pieBlurringSize / 2) - static_cast<float>(m_i);
  if(aa <= 0) return m_filled;
  if(aa > m_pieBlurringSize) return m_unfilled;
  // multiply by 100 because we are casting to integer
  return MixColorsInt(static_cast<long>(aa * 100.0f), static_cast<long>(m_pieBlurringSize * 100.0f), m_unfilled, m_filled);
}

void ProgressImages::InitializeProgressImages(CImageList& img, RgbPixel32 background, RgbPixel32 filled, RgbPixel32 unfilled)
{
  m_background = background;
  m_unfilled = unfilled;
  m_filled = filled;
  m_diameter = 14;
  m_radius = 7;
  m_pieBlurringSize = 2;
  m_perimeter = static_cast<int>(GetPI() * m_diameter);
  m_background = background;
  m_images.clear();
  m_images.reserve(m_perimeter+1);

  // add 1 for complete coverage, to be on the safe side.
  m_angles.Resize(m_radius + 1, static_cast<float>(m_perimeter), 0);// the values returned from m_angles will be in the range 0-perimeter, or in other words, 0-m_images.size().

  for(m_i = 0; m_i < m_perimeter; m_i ++)
  {
    m_bmp = new AnimBitmap<32>();
    m_bmp->SetSize(16, 16);
    m_bmp->Fill(m_background);
    FilledCircleAAG(8, 8, m_radius, this, DrawHLine, this, DrawAlphaPixel);
    // add it to the imagelist.
    HBITMAP hbm = m_bmp->DetachHandle();
    m_images.push_back(img.Add(hbm));
    DeleteObject(hbm);
    delete m_bmp;
    m_bmp = 0;
  }
}

int ProgressImages::GetImageFromProgress(int pos, int total)
{
  // pos / total = index / m_images.size();
  int index = (int)(0.05f + (float)pos * (float)m_images.size() / (float)total);
  // bounds checking.
  if(index < 0) index = 0;
  if(index >= (int)m_images.size()) index = m_images.size() - 1;
  return m_images[index];
}


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

    HICON hCheck = ::LoadIcon(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_CHECK));
    m_iconCheck = m_imageList.AddIcon(hCheck);
    DestroyIcon(hCheck);

    // set up the list view for messages
		m_listView = GetDlgItem(IDC_MESSAGES);
    
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
    case ITEM_GENERAL:
      break;
    case ITEM_FTP:
      menu.InsertMenuItem(pos ++, TRUE, MenuItemInfo::CreateText(_T("Copy URL"), ID_COPYURL));
      menu.InsertMenuItem(pos ++, TRUE, MenuItemInfo::CreateText(_T("Open URL..."), ID_OPENURL));
      break;
    case ITEM_FILE:
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

  try
	{
    Clipboard(m_hWnd).SetText(p->url);
	}
	catch (const Win32Exception& excp)
	{
#ifdef _DEBUG
		MessageBox(excp.What().c_str(), TEXT("Clipboard Error"),
			MB_OK | MB_ICONERROR);
#endif
	}
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


LPARAM CStatusDlg::AsyncCreateMessage(const MessageIcon icon, const MessageType type, const tstd::tstring& destination, const tstd::tstring& message, const tstd::tstring& url)
{
  LPARAM ret = 0;

	if (m_listView.IsWindow())
	{
		int itemID = m_listView.GetItemCount() + 1;
    ItemSpec* newSpec = new ItemSpec;
    ret = reinterpret_cast<LPARAM>(newSpec);

    newSpec->type = type;
    newSpec->url = url;

		itemID = m_listView.AddItem(itemID, 0, destination.c_str(), MessageIconToIconIndex(icon));
		m_listView.SetItemData(itemID, ret);
		m_listView.SetItemText(itemID, 1, message.c_str());

    m_listView.SetColumnWidth(0, LVSCW_AUTOSIZE);
    m_listView.SetColumnWidth(1, LVSCW_AUTOSIZE);
	}

  return ret;
}

void CStatusDlg::AsyncMessageSetIcon(LPARAM msgID, const MessageIcon icon)
{
  CriticalSection::ScopeLock lock(m_cs);
  int item;
  if(-1 != (item = MessageIDToItemID(msgID)))
  {
    m_listView.SetItem(item, 0, LVIF_IMAGE, 0, MessageIconToIconIndex(icon), 0, 0, 0);
  }
}

void CStatusDlg::AsyncMessageSetProgress(LPARAM msgID, int pos, int total)
{
  int item;
  if(-1 != (item = MessageIDToItemID(msgID)))
  {
    int iimage = m_progress.GetImageFromProgress(pos, total);
    if(pos >= total)
    {
      // 100% - use a special image.
      iimage = MessageIconToIconIndex(MSG_CHECK);
    }
    m_listView.SetItem(item, 0, LVIF_IMAGE, 0, iimage, 0, 0, 0);
  }
}

void CStatusDlg::AsyncMessageSetText(LPARAM msgID, const tstd::tstring& msg)
{
  int item;
  if(-1 != (item = MessageIDToItemID(msgID)))
  {
    m_listView.SetItemText(item, 1, msg.c_str());
  }
}

void CStatusDlg::AsyncMessageSetURL(LPARAM msgID, const tstd::tstring& url)
{
  ItemSpec* pItem = MessageIDToItemSpec(msgID);
  if(pItem)
  {
    pItem->url = url;
  }
}

int CStatusDlg::MessageIDToItemID(LPARAM msgID)
{
  LVFINDINFO fi = {0};
  fi.flags = LVFI_PARAM;
  fi.lParam = msgID;
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

CStatusDlg::ItemSpec* CStatusDlg::MessageIDToItemSpec(LPARAM msgID)
{
  if(-1 == MessageIDToItemID(msgID))
  {
    return 0;
  }
  return reinterpret_cast<ItemSpec*>(msgID);
}

CStatusDlg::ItemSpec* CStatusDlg::GetSelectedItemSpec()
{
  return ItemToItemSpec(m_listView.GetSelectedIndex());
}
