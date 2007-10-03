

#include "stdafx.hpp"
#include "ActivityList.hpp"
#include "image.hpp"
#include "Clipboard.hpp"
#include "MainWindow.hpp"
#include "libcc\timer.h"

void ActivityList::Attach(HWND h)
{
	m_ctl = h;
	SubclassWindow(::GetParent(h));

	// set up rendering info struct
	HDC hdc = GetDC();
	Gdiplus::Graphics* gfx = new Gdiplus::Graphics(hdc);
	Gdiplus::Font* font = new Gdiplus::Font(hdc, UtilGetShellFont());
	m_renderingInfo.m_lineHeight = (int)font->GetHeight(gfx);
	delete font;
	delete gfx;
	ReleaseDC(hdc);

	m_renderingInfo.m_hwnd = h;

	// create and populate image list for message icons
	m_renderingInfo.m_iconHeight = 16;
	m_renderingInfo.m_iconWidth = 16;

	if (m_renderingInfo.m_imageList.Create(m_renderingInfo.m_iconWidth, m_renderingInfo.m_iconHeight, ILC_COLOR32, 2, 0))
	{
		m_renderingInfo.m_iconInfo = m_renderingInfo.m_imageList.AddIcon(::LoadIcon(NULL, IDI_INFORMATION));
    m_renderingInfo.m_iconWarning = m_renderingInfo.m_imageList.AddIcon(::LoadIcon(NULL, IDI_WARNING));
		m_renderingInfo.m_iconError = m_renderingInfo.m_imageList.AddIcon(::LoadIcon(NULL, IDI_ERROR));

    HICON hCheck = (HICON)::LoadImage(_Module.GetResourceInstance(),
			MAKEINTRESOURCE(IDI_CHECK), IMAGE_ICON, 16, 16, 0);
    m_renderingInfo.m_iconCheck = m_renderingInfo.m_imageList.AddIcon(hCheck);
    DestroyIcon(hCheck);
	}

  m_renderingInfo.m_progress.InitializeProgressImages(m_renderingInfo.m_imageList,
    COLORREFToRgbPixel32(GetSysColor(COLOR_WINDOW)),
    MakeRgbPixel32(222,123,16),
    MakeRgbPixel32(0,40,86));

	LibCC::Timer t;
	t.Tick();

	if(NULL == GetProp(m_ctl, _T("InitialPopulation")))
	{
		// populate the initial thingy
		std::vector<ScreenshotArchive::Screenshot> screenshots = m_archive.RetreiveScreenshots();
		for(std::vector<ScreenshotArchive::Screenshot>::iterator it = screenshots.begin(); it != screenshots.end(); ++ it)
		{
			CallInternalRegisterScreenshot(it->id, it->RetrieveThumbnail(), it->width, it->height);

			std::vector<ScreenshotArchive::Event> events = it->RetreiveEvents();
			for(std::vector<ScreenshotArchive::Event>::iterator itEvent = events.begin(); itEvent != events.end(); ++ itEvent)
			{
				CallInternalRegisterEvent(itEvent->id, itEvent->screenshotID, itEvent->icon, itEvent->type, itEvent->destinationName, itEvent->messageText, itEvent->url);
			}
		}
		SetProp(m_ctl, _T("InitialPopulation"), (HANDLE)1);
	}

	t.Tick();
	LibCC::g_pLog->Message(LibCC::Format("Initial history population performed in % seconds")(t.GetLastDelta()));
}

// IArchiveNotifications events
void ActivityList::OnPruneScreenshot(ScreenshotID screenshotID)// screenshotID is the archives's screenshotID
{
	InternalDeleteScreenshot(screenshotID);
}


ScreenshotID ActivityList::RegisterScreenshot(Gdiplus::BitmapPtr image, Gdiplus::BitmapPtr thumbnail)
{
	ScreenshotID ret = m_archive.RegisterScreenshot(image, thumbnail);
	CallInternalRegisterScreenshot(ret, thumbnail, image->GetWidth(), image->GetHeight());
	return ret;
}

EventID ActivityList::RegisterEvent(ScreenshotID screenshotID, EventIcon icon, EventType type, const std::wstring& destination, const std::wstring& message, const std::wstring& url)
{
	EventID ret = m_archive.RegisterEvent(screenshotID, icon, type, destination, message, url);
	CallInternalRegisterEvent(ret, screenshotID, icon, type, destination, message, url);
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

// make sure the list index is correct.
void ActivityList::RedrawItem(ActivityListItemSpec* i)
{
	// UMMM how the F do you tell the listbox to redraw a single item? i guess this is the only way
	RECT rc;
	if(LB_ERR == m_ctl.GetItemRect(i->GetListIndex(), &rc))
		return;
	m_ctl.InvalidateRect(&rc, FALSE);
}

LRESULT ActivityList::OnInternalRegisterScreenshot(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	IRS_Data* p = (IRS_Data*)lParam;
	_InternalRegisterScreenshot(p->archiveScreenshotID, p->thumbnail, p->screenshotWidth, p->screenshotHeight);
	delete p;
	return 0;
}

LRESULT ActivityList::OnInternalRegisterEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	IRE_Data* p = (IRE_Data*)lParam;
	_InternalRegisterEvent(p->archiveEventID, p->screenshotID, p->icon, p->type, p->destination, p->message, p->url);
	delete p;
	return 0;
}

void ActivityList::CallInternalRegisterScreenshot(ScreenshotID archiveScreenshotID, Gdiplus::BitmapPtr thumbnail, int screenshotWidth, int screenshotHeight)
{
	if(this->GetWindowThreadID() == GetCurrentThreadId())
	{
		_InternalRegisterScreenshot(archiveScreenshotID, thumbnail, screenshotWidth, screenshotHeight);
		return;
	}
	IRS_Data* p = new IRS_Data;
	p->archiveScreenshotID = archiveScreenshotID;
	p->screenshotHeight = screenshotHeight;
	p->screenshotWidth = screenshotWidth;
	p->thumbnail = Gdiplus::BitmapPtr(thumbnail->Clone(0, 0, thumbnail->GetWidth(), thumbnail->GetHeight(), thumbnail->GetPixelFormat()));
	SendMessage(WM_INTERNALREGISTERSCREENSHOT, 0, (LPARAM)p);
}

void ActivityList::CallInternalRegisterEvent(EventID archiveEventID, ScreenshotID screenshotID, EventIcon icon, EventType type, const std::wstring& destination, const std::wstring& message, const std::wstring& url)
{
	if(this->GetWindowThreadID() == GetCurrentThreadId())
	{
		_InternalRegisterEvent(archiveEventID, screenshotID, icon, type, destination, message, url);
		return;
	}
	IRE_Data* p = new IRE_Data;
	p->archiveEventID = archiveEventID;
	p->destination = destination;
	p->icon = icon;
	p->message = message;
	p->screenshotID = screenshotID;
	p->type = type;
	p->url = url;
	SendMessage(WM_INTERNALREGISTEREVENT, 0, (LPARAM)p);
}

// this must run in the GUI thread.
void ActivityList::_InternalRegisterScreenshot(ScreenshotID archiveScreenshotID, Gdiplus::BitmapPtr thumbnail, int screenshotWidth, int screenshotHeight)
{
	ATLASSERT(m_currentlyInsertingItem == 0);
	ATLASSERT(GetWindowThreadID() == GetCurrentThreadId());

	m_items.push_back(ActivityListItemSpec());
	ActivityListItemSpec& pnew = m_items.back();
	pnew.Init(&m_renderingInfo);
	pnew.SetType(ActivityListItemSpec::Screenshot);
	pnew.SetScreenshotID(archiveScreenshotID);
	pnew.SetThumbnail(thumbnail);
	pnew.SetScreenshotWidth(screenshotWidth);
	pnew.SetScreenshotHeight(screenshotHeight);

	m_currentlyInsertingItem = &pnew;
	{
		LibCC::LogScopeMessage lsm(LibCC::Format(L"Inserting item %").p(&pnew).Str());
		int itemID = m_ctl.AddString(_T("."));
		LibCC::g_pLog->Message(LibCC::Format("... index %").i(itemID));
		m_currentlyInsertingItem = 0;
		m_ctl.SetItemDataPtr(itemID, &pnew);
		m_ctl.SetTopIndex(itemID);
	}
}

// this must run in the GUI thread.
void ActivityList::_InternalRegisterEvent(EventID archiveEventID, ScreenshotID screenshotID, EventIcon icon, EventType type, const std::wstring& destination, const std::wstring& message, const std::wstring& url)
{
	ATLASSERT(m_currentlyInsertingItem == 0);
	ATLASSERT(GetWindowThreadID() == GetCurrentThreadId());

	m_items.push_back(ActivityListItemSpec());
	ActivityListItemSpec& pnew = m_items.back();
	pnew.Init(&m_renderingInfo);
	pnew.SetType(ActivityListItemSpec::Event);
	pnew.SetScreenshotID(screenshotID);
	pnew.SetEventType(type);
	pnew.SetEventID(archiveEventID);
	pnew.SetEventDestination(destination);
	pnew.SetEventIcon(icon);
	pnew.SetEventMessage(message);
	pnew.SetEventURL(url);
	pnew.SetEventImageIndex(m_renderingInfo.EventIconToIconIndex(icon));

	m_currentlyInsertingItem = &pnew;
	{
		LibCC::LogScopeMessage lsm(LibCC::Format(L"Inserting item %").p(&pnew).Str());
		int itemID = m_ctl.AddString(_T("."));
		LibCC::g_pLog->Message(LibCC::Format("... index %").i(itemID));
		m_currentlyInsertingItem = 0;
		m_ctl.SetItemDataPtr(itemID, &pnew);
		m_ctl.SetTopIndex(itemID);
	}
}

void ActivityList::InternalEventSetIcon(EventID eventID, EventIcon icon)
{
	ActivityListItemSpec* item = EventIDToItemSpecWithListIndex(eventID);
	if(!item)
		return;
	item->SetEventIcon(icon);
	item->SetEventImageIndex(m_renderingInfo.EventIconToIconIndex(icon));

	RedrawItem(item);
}

void ActivityList::InternalEventSetProgress(EventID eventID, int pos, int total)
{
	ActivityListItemSpec* item = EventIDToItemSpecWithListIndex(eventID);
	if(!item)
		return;
	item->SetEventImageIndex(m_renderingInfo.m_progress.GetImageFromProgress(pos, total));
	if(pos >= total)
	{
		// 100% - use a special image.
		item->SetEventImageIndex(m_renderingInfo.EventIconToIconIndex(EI_CHECK));
	}
	RedrawItem(item);
}

void ActivityList::InternalEventSetText(EventID eventID, const std::wstring& msg)
{
	ActivityListItemSpec* item = EventIDToItemSpecWithListIndex(eventID);
	if(!item)
		return;
	item->SetEventMessage(msg);
	RedrawItem(item);
}

void ActivityList::InternalEventSetURL(EventID eventID, const std::wstring& url)
{
	ActivityListItemSpec* item = EventIDToItemSpecWithListIndex(eventID);
	if(!item)
		return;
	item->SetEventURL(url);
	RedrawItem(item);
}

void ActivityList::InternalDeleteEvent(EventID eventID)
{
	ActivityListItemSpec* item = EventIDToItemSpecWithListIndex(eventID);
	if(!item)
		return;
	m_ctl.DeleteString(item->GetListIndex());
}

void ActivityList::InternalDeleteScreenshot(ScreenshotID screenshotID)
{
	ActivityListItemSpec* item;
	while(item = GetFirstItemAssociatedWithScreenshot(screenshotID))
	{
		m_ctl.DeleteString(item->GetListIndex());
	}
}


LRESULT ActivityList::OnDeleteItem(DELETEITEMSTRUCT& dis, BOOL& bHandled)
{
	bHandled = TRUE;
	ActivityListItemSpec* item = (ActivityListItemSpec*)dis.itemData;

	// just remove it from the list.
	// so, first find it.
	for(std::list<ActivityListItemSpec>::iterator it = m_items.begin(); it != m_items.end(); ++ it)
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
				std::vector<ActivityListItemSpec*> items = GetSelectedItems();
				for(std::vector<ActivityListItemSpec*>::iterator it = items.begin(); it != items.end(); ++ it)
				{
					m_ctl.SetSel((*it)->GetListIndex(), FALSE);
				}
			}
			m_ctl.SetSel(i);
		}
	}

	ActivityListItemSpec* spec = GetFocusedItem();
  if(!spec)
		return 0;

	CMenu menu;
  menu.CreatePopupMenu();
  int pos = 0;

  if(spec)
  {
		switch(spec->GetType())
		{
		case ActivityListItemSpec::Screenshot:
				menu.InsertMenuItem(pos ++, TRUE, MenuItemInfo::CreateText(_T("Re-process..."), ID_REPROCESS));
			break;
		case ActivityListItemSpec::Event:
			{
				switch(spec->GetEventType())
				{
				default:
				case ET_GENERAL:
					if(spec->GetEventURL().size())
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
	if(spec->GetEventMessage().size())
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
	ActivityListItemSpec* item = GetFocusedItem();
	if(!item)
		return 0;
  tstd::tstring cmdLine = LibCC::Format("explorer /select, %").qs(item->GetEventURL()).Str();

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
	ActivityListItemSpec* item = GetFocusedItem();
	if(!item)
		return 0;
  ShellExecute(m_hWnd, _T("open"), item->GetEventURL().c_str(), NULL, NULL, SW_SHOW);
  return 0;
}

LRESULT ActivityList::OnOpenURL(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ActivityListItemSpec* item = GetFocusedItem();
	if(!item)
		return 0;
  ShellExecute(m_hWnd, _T("open"), item->GetEventURL().c_str(), NULL, NULL, SW_SHOW);
  return 0;
}

LRESULT ActivityList::OnCopyURL(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ActivityListItemSpec* item = GetFocusedItem();
	if(!item)
		return 0;

	LibCC::Result r = Clipboard(m_hWnd).SetText(item->GetEventURL());
	return 0;
}

LRESULT ActivityList::OnCopyMessage(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ActivityListItemSpec* item = GetFocusedItem();
	if(!item)
		return 0;

	LibCC::Result r = Clipboard(m_hWnd).SetText(item->GetEventMessage());
	return 0;
}

LRESULT ActivityList::OnClear(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(IDYES != MessageBox(L"Are you sure you want to delete all items from your history?", L"Screenie", MB_YESNO | MB_ICONQUESTION))
		return 0;

	m_archive.DeleteAll();
	m_ctl.ResetContent();
  return 0;
}

// remove selected.
LRESULT ActivityList::OnRemove(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	std::vector<ActivityListItemSpec*> items = GetSelectedItems();

	std::wstring caption = L"Are you sure you want to delete these items from your history?";
	if(items.size() == 1)
	{
		caption = L"Are you sure you want to delete this item from your history?";
	}
	if(IDYES != MessageBox(caption.c_str(), L"Screenie", MB_YESNO | MB_ICONQUESTION))
		return 0;

	while(ActivityListItemSpec* item = GetFirstSelectedItem())
	{
		switch(item->GetType())
		{
		case ActivityListItemSpec::Screenshot:
			DeleteScreenshot(item->GetScreenshotID());
			break;
		case ActivityListItemSpec::Event:
			DeleteEvent(item->GetEventID());
			break;
		}
	}
	return 0;
}

extern CMainWindow* g_mainWindow;

LRESULT ActivityList::OnReprocess(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	ActivityListItemSpec* item = GetFocusedItem();
	if(!item)
		return 0;
	::PostMessage(*g_mainWindow, CMainWindow::WM_REPROCESSSCREENSHOT, 0, (LPARAM)item->GetScreenshotID());
	return 0;
}

ActivityListItemSpec* ActivityList::GetFocusedItem() const
{
	LibCC::Blob<INT> x(m_ctl.GetSelCount());
	m_ctl.GetSelItems(x.Size(), x.GetBuffer());
	for(size_t i = 0; i < x.Size(); ++ i)
	{
		if(0 != m_ctl.GetSel(x[i]))
			return (ActivityListItemSpec*)m_ctl.GetItemDataPtr(x[i]);
	}
	return 0;
}

std::vector<ActivityListItemSpec*> ActivityList::GetSelectedItems() const
{
	LibCC::Blob<INT> x(m_ctl.GetSelCount());
	m_ctl.GetSelItems(x.Size(), x.GetBuffer());
	std::vector<ActivityListItemSpec*> ret;
	for(size_t i = 0; i < x.Size(); ++ i)
	{
		ActivityListItemSpec* p = (ActivityListItemSpec*)m_ctl.GetItemDataPtr(x[i]);
		p->SetListIndex(x[i]);
		ret.push_back(p);
	}
	return ret;
}

ActivityListItemSpec* ActivityList::EventIDToItemSpecWithListIndex(EventID id)
{
	// it would be nice if there was an easier way of doing this.
	for(int i = 0; i < m_ctl.GetCount(); i ++)
	{
		ActivityListItemSpec* p = (ActivityListItemSpec*)m_ctl.GetItemDataPtr(i);
		if(p->GetType() == ActivityListItemSpec::Event && p->GetEventID() == id)
		{
			p->SetListIndex(i);
			return p;
		}
	}
	return 0;
}

ActivityListItemSpec* ActivityList::GetFirstItemAssociatedWithScreenshot(ScreenshotID screenshotID) const
{
	// it would be nice if there was an easier way of doing this.
	for(int i = 0; i < m_ctl.GetCount(); i ++)
	{
		ActivityListItemSpec* p = (ActivityListItemSpec*)m_ctl.GetItemDataPtr(i);
		if(p->GetScreenshotID() == screenshotID)
		{
			p->SetListIndex(i);
			return p;
		}
	}
	return 0;
}

ActivityListItemSpec* ActivityList::GetFirstSelectedItem() const
{
	if(m_ctl.GetSelCount() == 0)
		return 0;
	INT i[2];
	m_ctl.GetSelItems(1, i);
	return (ActivityListItemSpec*)m_ctl.GetItemDataPtr(i[0]);
}


LRESULT ActivityList::OnDrawItem(DRAWITEMSTRUCT& dis, BOOL& bHandled)
{
	bHandled = TRUE;
	if(dis.itemID == -1)
	{
		return 0;
	}

	ActivityListItemSpec* item = (ActivityListItemSpec*)dis.itemData;
	//LibCC::g_pLog->Message(LibCC::Format("OnDrawItem. Index=% p=%").i(dis.itemID).p(item));
	item->RenderTo(dis.hDC, dis.rcItem, (dis.itemState & ODS_SELECTED) == ODS_SELECTED);
	return 0;
}

LRESULT ActivityList::OnMeasureItem(MEASUREITEMSTRUCT& mis, BOOL& bHandled)
{
	bHandled = TRUE;

	ActivityListItemSpec* item;
	if(m_currentlyInsertingItem)
		item = m_currentlyInsertingItem;
	else
		item = (ActivityListItemSpec*)m_ctl.GetItemDataPtr(mis.itemID);

	mis.itemHeight = item->GetItemHeight();

	return 0;
}
