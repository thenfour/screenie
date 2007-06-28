

#include "stdafx.hpp"
#include "ActivityList.hpp"
#include "geom.h"
#include "image.hpp"
#include "Clipboard.hpp"
#include "MainWindow.hpp"


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
	FilledCircleAAG(8, 8, m_radius, this, &ProgressImages::DrawHLine, this, &ProgressImages::DrawAlphaPixel);
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

void ActivityList::Attach(HWND h)
{
	m_ctl = h;

	SubclassWindow(::GetParent(h));

	HDC hdc = GetDC();
	Gdiplus::Graphics* gfx = new Gdiplus::Graphics(hdc);
	Gdiplus::Font* font = new Gdiplus::Font(hdc, UtilGetShellFont());
	m_itemHeight = (int)font->GetHeight(gfx);
	delete font;
	delete gfx;
	ReleaseDC(hdc);

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
	}

  m_progress.InitializeProgressImages(m_imageList,
    COLORREFToRgbPixel32(GetSysColor(COLOR_WINDOWTEXT)),
    MakeRgbPixel32(222,123,16),
    MakeRgbPixel32(0,40,86));


	if(NULL == GetProp(m_ctl, _T("InitialPopulation")))
	{
		// populate the initial thingy
		std::vector<ScreenshotArchive::Screenshot> screenshots = m_archive.RetreiveScreenshots();
		for(std::vector<ScreenshotArchive::Screenshot>::iterator it = screenshots.begin(); it != screenshots.end(); ++ it)
		{
			InternalRegisterScreenshot(it->id, it->RetrieveThumbnail());

			std::vector<ScreenshotArchive::Event> events = it->RetreiveEvents();
			for(std::vector<ScreenshotArchive::Event>::iterator itEvent = events.begin(); itEvent != events.end(); ++ itEvent)
			{
				InternalRegisterEvent(itEvent->id, itEvent->screenshotID, itEvent->icon, itEvent->type, itEvent->destinationName, itEvent->messageText, itEvent->url);
			}
		}
		SetProp(m_ctl, _T("InitialPopulation"), (HANDLE)1);
	}

	m_brushSelected.CreateSysColorBrush(COLOR_HIGHLIGHT);
	m_brushNormal.CreateSysColorBrush(COLOR_WINDOW);
}

// IArchiveNotifications events
void ActivityList::OnPruneScreenshot(ScreenshotID screenshotID)// screenshotID is the archives's screenshotID
{
	InternalDeleteScreenshot(screenshotID);
}


ScreenshotID ActivityList::RegisterScreenshot(Gdiplus::BitmapPtr image, Gdiplus::BitmapPtr thumbnail)
{
	ScreenshotID ret = m_archive.RegisterScreenshot(image, thumbnail);
	InternalRegisterScreenshot(ret, thumbnail);
	return ret;
}

EventID ActivityList::RegisterEvent(ScreenshotID screenshotID, EventIcon icon, EventType type, const std::wstring& destination, const std::wstring& message, const std::wstring& url)
{
	EventID ret = m_archive.RegisterEvent(screenshotID, icon, type, destination, message, url);
	InternalRegisterEvent(ret, screenshotID, icon, type, destination, message, url);
	return ret;
}

void ActivityList::EventSetIcon(EventID eventID, EventIcon icon)
{
	m_archive.EventSetIcon(eventID, icon);
	InternalEventSetIcon(eventID, icon);
}

void ActivityList::EventSetProgress(EventID eventID, int pos, int total)
{
	m_archive.EventSetProgress(eventID, pos, total);
	InternalEventSetProgress(eventID, pos, total);
}

void ActivityList::EventSetText(EventID eventID, const std::wstring& msg)
{
	m_archive.EventSetText(eventID, msg);
	InternalEventSetText(eventID, msg);
}

void ActivityList::EventSetURL(EventID eventID, const std::wstring& url)
{
	m_archive.EventSetURL(eventID, url);
	InternalEventSetURL(eventID, url);
}

void ActivityList::DeleteEvent(EventID eventID)
{
	m_archive.DeleteEvent(eventID);
	InternalDeleteEvent(eventID);
}

void ActivityList::DeleteScreenshot(ScreenshotID screenshotID)
{
	m_archive.DeleteScreenshot(screenshotID);
	InternalDeleteScreenshot(screenshotID);
}


void ActivityList::InternalRegisterScreenshot(ScreenshotID archiveScreenshotID, Gdiplus::BitmapPtr thumbnail)
{
	m_items.push_back(ItemSpec());
	ItemSpec& pnew = m_items.back();
	pnew.type = ItemSpec::Screenshot;
	pnew.screenshotID = archiveScreenshotID;
	pnew.thumb = thumbnail;

	m_currentlyInsertingItem = &pnew;
	int itemID = m_ctl.AddString(_T("."));
	m_currentlyInsertingItem = 0;
	m_ctl.SetItemDataPtr(itemID, &pnew);
}

void ActivityList::InternalRegisterEvent(EventID archiveEventID, ScreenshotID screenshotID, EventIcon icon, EventType type, const std::wstring& destination, const std::wstring& message, const std::wstring& url)
{
	m_items.push_back(ItemSpec());
	ItemSpec& pnew = m_items.back();
	pnew.type = ItemSpec::Event;
	pnew.screenshotID = screenshotID;
	pnew.eventType = type;
	pnew.eventID = archiveEventID;
	pnew.destination = destination;
	pnew.icon = icon;
	pnew.msg = message;
	pnew.url = url;
	pnew.imageIndex = EventIconToIconIndex(icon);

	m_currentlyInsertingItem = &pnew;
	int itemID = m_ctl.AddString(_T("."));
	m_currentlyInsertingItem = 0;
	m_ctl.SetItemDataPtr(itemID, &pnew);
}

void ActivityList::InternalEventSetIcon(EventID eventID, EventIcon icon)
{
	ItemSpec* item = EventIDToItemSpec(eventID);
	if(!item)
		return;
	item->icon = icon;
	item->imageIndex = EventIconToIconIndex(icon);
}

void ActivityList::InternalEventSetProgress(EventID eventID, int pos, int total)
{
	ItemSpec* item = EventIDToItemSpec(eventID);
	if(!item)
		return;
	item->imageIndex = m_progress.GetImageFromProgress(pos, total);
	if(pos >= total)
	{
		// 100% - use a special image.
		item->imageIndex = EventIconToIconIndex(EI_CHECK);
	}
}

void ActivityList::InternalEventSetText(EventID eventID, const std::wstring& msg)
{
	ItemSpec* item = EventIDToItemSpec(eventID);
	if(!item)
		return;
	item->msg = msg;
}

void ActivityList::InternalEventSetURL(EventID eventID, const std::wstring& url)
{
	ItemSpec* item = EventIDToItemSpec(eventID);
	if(!item)
		return;
	item->url = url;
}

void ActivityList::InternalDeleteEvent(EventID eventID)
{
	ItemSpec* item = EventIDToItemSpecWithListIndex(eventID);
	if(!item)
		return;
	// delete this item.
	m_ctl.DeleteString(item->listIndex);
}

void ActivityList::InternalDeleteScreenshot(ScreenshotID screenshotID)
{
	ItemSpec* item;
	while(item = GetFirstItemAssociatedWithScreenshot(screenshotID))
	{
		m_ctl.DeleteString(item->listIndex);
	}
}

ActivityList::ItemSpec* ActivityList::EventIDToItemSpec(EventID msgID)
{
	for(std::list<ItemSpec>::iterator it = m_items.begin(); it != m_items.end(); ++ it)
	{
		if(it->type == ItemSpec::Event && it->eventID == msgID)
			return &(*it);
	}
	return 0;
}

ActivityList::ItemSpec* ActivityList::ScreenshotIDToItemSpec(ScreenshotID screenshotID)
{
	for(std::list<ItemSpec>::iterator it = m_items.begin(); it != m_items.end(); ++ it)
	{
		if(it->type == ItemSpec::Screenshot && it->screenshotID == screenshotID)
			return &(*it);
	}
	return 0;
}

LRESULT ActivityList::OnDrawItem(DRAWITEMSTRUCT& dis, BOOL& bHandled)
{
	bHandled = TRUE;
	if(dis.itemID == -1) return 0;

	ItemSpec& item = *((ItemSpec*)dis.itemData);

	SetBkMode(dis.hDC, TRANSPARENT);

	if((dis.itemState & ODS_SELECTED) == ODS_SELECTED)
	{
		FillRect(dis.hDC, &dis.rcItem, m_brushSelected);
		SetTextColor(dis.hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
	}
	else
	{
		FillRect(dis.hDC, &dis.rcItem, m_brushNormal);
		SetTextColor(dis.hDC, GetSysColor(COLOR_WINDOWTEXT));
	}

	SelectObject(dis.hDC, UtilGetShellFont());

	switch(item.type)
	{
	case ItemSpec::Screenshot:
		{
			Gdiplus::Graphics* p = new Gdiplus::Graphics(dis.hDC);
			p->DrawImage(item.thumb.get(), dis.rcItem.left, dis.rcItem.top);
			delete p;
		}
		break;
	case ItemSpec::Event:
		DrawText(dis.hDC, item.msg.c_str(), item.msg.size(), &dis.rcItem, 0);
		break;
	default:
		// no man's land.
		break;
	}

	return 0;
}

LRESULT ActivityList::OnMeasureItem(MEASUREITEMSTRUCT& mis, BOOL& bHandled)
{
	bHandled = TRUE;

	ItemSpec* item;
	if(m_currentlyInsertingItem)
		item = m_currentlyInsertingItem;
	else
		item = (ItemSpec*)m_ctl.GetItemDataPtr(mis.itemID);

	switch(item->type)
	{
	case ItemSpec::Screenshot:
		mis.itemHeight = item->thumb->GetHeight();
		break;
	case ItemSpec::Event:
		mis.itemHeight = m_itemHeight;
		break;
	default:
		// no-man's land.
		mis.itemHeight = 100;
		break;
	}
	return 0;
}

LRESULT ActivityList::OnDeleteItem(DELETEITEMSTRUCT& dis, BOOL& bHandled)
{
	bHandled = TRUE;
	ItemSpec* item = (ItemSpec*)dis.itemData;
	// just remove it from the list.
	// so, first find it.
	for(std::list<ItemSpec>::iterator it = m_items.begin(); it != m_items.end(); ++ it)
	{
		if(&(*it) == item)
		{
			m_items.erase(it);
			break;
		}
	}
	return 0;
}

LRESULT ActivityList::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if((HWND)wParam != m_ctl.m_hWnd)
		return 0;

	{
		// list boxes do not recognize right-clicks the same way as a listview (where a rightclick will also select the item)
		// so try and fake it.
		BOOL outside = FALSE;
		CPoint pt(LOWORD(lParam), HIWORD(lParam));
		m_ctl.ScreenToClient(&pt);
		UINT i = m_ctl.ItemFromPoint(pt, outside);
		if(TRUE == outside)
			return 0;

		if(0 == m_ctl.GetSel(i))
		{
			if(0 == GetAsyncKeyState(VK_CONTROL))
			{
				// de-select everything else
				std::vector<ItemSpec*> items = GetSelectedItems();
				for(std::vector<ItemSpec*>::iterator it = items.begin(); it != items.end(); ++ it)
				{
					m_ctl.SetSel((*it)->listIndex, FALSE);
				}
			}
			m_ctl.SetSel(i);
		}
	}

	ItemSpec* spec = GetFocusedItem();
  if(!spec)
		return 0;

	CMenu menu;
  menu.CreatePopupMenu();
  int pos = 0;

  if(spec)
  {
		switch(spec->type)
		{
		case ItemSpec::Screenshot:
				menu.InsertMenuItem(pos ++, TRUE, MenuItemInfo::CreateText(_T("Re-process..."), ID_REPROCESS));
			break;
		case ItemSpec::Event:
			{
				switch(spec->eventType)
				{
				default:
				case ET_GENERAL:
					if(spec->url.size())
						menu.InsertMenuItem(pos ++, TRUE, MenuItemInfo::CreateText(_T("Copy URL"), ID_COPYURL));
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
				break;
			}
		default:
			// no man's land.
			break;
		}
  }  

	// append global items
	if(pos > 0)
	{
		menu.InsertMenuItem(pos ++, TRUE, MenuItemInfo::CreateSeparator());
	}
	if(spec->msg.size())
		menu.InsertMenuItem(pos ++, TRUE, MenuItemInfo::CreateText(_T("Copy message text"), ID_COPYMESSAGE));
	menu.InsertMenuItem(pos ++, TRUE, MenuItemInfo::CreateText(_T("Remove"), IDC_REMOVE));
	menu.InsertMenuItem(pos ++, TRUE, MenuItemInfo::CreateText(_T("Clear all"), ID_CLEAR));

	POINT cursorPos = { 0 };
	::GetCursorPos(&cursorPos);
	menu.TrackPopupMenu(0, cursorPos.x, cursorPos.y, m_hWnd);

	return 0;
}

LRESULT ActivityList::OnExplore(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ItemSpec* item = GetFocusedItem();
	if(!item)
		return 0;
  tstd::tstring cmdLine = LibCC::Format("explorer /select, %").qs(item->url).Str();

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

LRESULT ActivityList::OnOpenFile(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ItemSpec* item = GetFocusedItem();
	if(!item)
		return 0;
  ShellExecute(m_hWnd, _T("open"), item->url.c_str(), NULL, NULL, SW_SHOW);
  return 0;
}

LRESULT ActivityList::OnOpenURL(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ItemSpec* item = GetFocusedItem();
	if(!item)
		return 0;
  ShellExecute(m_hWnd, _T("open"), item->url.c_str(), NULL, NULL, SW_SHOW);
  return 0;
}

LRESULT ActivityList::OnCopyURL(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ItemSpec* item = GetFocusedItem();
	if(!item)
		return 0;

	LibCC::Result r = Clipboard(m_hWnd).SetText(item->url);
	return 0;
}

LRESULT ActivityList::OnCopyMessage(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ItemSpec* item = GetFocusedItem();
	if(!item)
		return 0;

	LibCC::Result r = Clipboard(m_hWnd).SetText(item->msg);
	return 0;
}

LRESULT ActivityList::OnClear(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	m_archive.DeleteAll();
	m_ctl.ResetContent();
  return 0;
}

// remove selected.
LRESULT ActivityList::OnRemove(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	std::vector<ItemSpec*> items = GetSelectedItems();
	while(ItemSpec* item = GetFirstSelectedItem())
	{
		switch(item->type)
		{
		case ItemSpec::Screenshot:
			DeleteScreenshot(item->screenshotID);
			break;
		case ItemSpec::Event:
			DeleteEvent(item->eventID);
			break;
		}
	}
	return 0;
}

extern CMainWindow* g_mainWindow;

LRESULT ActivityList::OnReprocess(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ItemSpec* item = GetFocusedItem();
	if(!item)
		return 0;
	::PostMessage(*g_mainWindow, CMainWindow::WM_REPROCESSSCREENSHOT, 0, (LPARAM)item->screenshotID);
	return 0;
}

ActivityList::ItemSpec* ActivityList::GetFocusedItem() const
{
	LibCC::Blob<INT> x(m_ctl.GetSelCount());
	m_ctl.GetSelItems(x.Size(), x.GetBuffer());
	for(size_t i = 0; i < x.Size(); ++ i)
	{
		if(0 != m_ctl.GetSel(x[i]))
			return (ItemSpec*)m_ctl.GetItemDataPtr(x[i]);
	}
	return 0;
}

std::vector<ActivityList::ItemSpec*> ActivityList::GetSelectedItems() const
{
	LibCC::Blob<INT> x(m_ctl.GetSelCount());
	m_ctl.GetSelItems(x.Size(), x.GetBuffer());
	std::vector<ItemSpec*> ret;
	for(size_t i = 0; i < x.Size(); ++ i)
	{
		ItemSpec* p = (ItemSpec*)m_ctl.GetItemDataPtr(x[i]);
		p->listIndex = x[i];
		ret.push_back(p);
	}
	return ret;
}

ActivityList::ItemSpec* ActivityList::EventIDToItemSpecWithListIndex(EventID id)
{
	for(int i = 0; i < m_ctl.GetCount(); i ++)
	{
		ItemSpec* p = (ItemSpec*)m_ctl.GetItemDataPtr(i);
		if(p->type == ItemSpec::Event && p->eventID == id)
		{
			p->listIndex = i;
			return p;
		}
	}
	return 0;
}

ActivityList::ItemSpec* ActivityList::GetFirstItemAssociatedWithScreenshot(ScreenshotID screenshotID) const
{
	for(int i = 0; i < m_ctl.GetCount(); i ++)
	{
		ItemSpec* p = (ItemSpec*)m_ctl.GetItemDataPtr(i);
		if(p->screenshotID == screenshotID)
		{
			p->listIndex = i;
			return p;
		}
	}
	return 0;
}

ActivityList::ItemSpec* ActivityList::GetFirstSelectedItem() const
{
	if(m_ctl.GetSelCount() == 0)
		return 0;
	INT i[2];
	m_ctl.GetSelItems(1, i);
	return (ItemSpec*)m_ctl.GetItemDataPtr(i[0]);
}

