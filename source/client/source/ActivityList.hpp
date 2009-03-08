// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark

#ifndef SCREENIE_ACTIVITYLIST_HPP
#define SCREENIE_ACTIVITYLIST_HPP

#include "animbitmap.h"
#include "resource.h"
#include "ScreenshotArchive.hpp"
#include "ActivityListItem.hpp"
#include <list>

struct IRS_Data
{
	ScreenshotID archiveScreenshotID;
	Gdiplus::BitmapPtr thumbnail;
	int screenshotWidth;
	int screenshotHeight;
};

struct IRE_Data
{
	EventID archiveEventID; 
	ScreenshotID screenshotID;
	EventIcon icon;
	EventType type;
	std::wstring destination;
	std::wstring message;
	std::wstring url;
};

class ActivityList :
  public CWindowImpl<ActivityList>,
	public IActivity,
	public IArchiveNotifications
{
public:
  static const short ID_COPYURL = 4000;
  static const short ID_COPYMESSAGE = 4001;
  static const short ID_CLEAR = 4002;
  static const short ID_EXPLORE = 4003;
  static const short ID_OPENFILE = 4004;
  static const short ID_OPENURL = 4005;
  static const short ID_REPROCESS = 4006;
  static const UINT WM_INTERNALREGISTERSCREENSHOT = WM_APP + 700;
  static const UINT WM_INTERNALREGISTEREVENT = WM_APP + 701;

	ActivityList(ScreenshotArchive& archive, ScreenshotOptions& options) :
		m_archive(archive),
		m_options(options),
		m_currentlyInsertingItem(0)
	{
	}

	BEGIN_MSG_MAP(ActivityList)
    COMMAND_ID_HANDLER(IDC_CLEAR, OnClear)
    COMMAND_ID_HANDLER(IDC_REMOVE, OnRemove)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)

		MESSAGE_HANDLER(WM_INTERNALREGISTERSCREENSHOT, OnInternalRegisterScreenshot)
		MESSAGE_HANDLER(WM_INTERNALREGISTEREVENT, OnInternalRegisterEvent)

    COMMAND_ID_HANDLER(ID_REPROCESS, OnReprocess)
    COMMAND_ID_HANDLER(ID_COPYURL, OnCopyURL)
    COMMAND_ID_HANDLER(ID_COPYMESSAGE, OnCopyMessage)
    COMMAND_ID_HANDLER(ID_CLEAR, OnClear)
    COMMAND_ID_HANDLER(ID_EXPLORE, OnExplore)
    COMMAND_ID_HANDLER(ID_OPENFILE, OnOpenFile)
    COMMAND_ID_HANDLER(ID_OPENURL, OnOpenURL)
	END_MSG_MAP()

	void Attach(HWND h);

	virtual ~ActivityList()
	{
	}

	// IActivity. These are called by the status dlg, and basically just forward to the internal versions + forward to the archive.
	ScreenshotID RegisterScreenshot(Gdiplus::BitmapPtr image, Gdiplus::BitmapPtr thumbnail);
	EventID RegisterEvent(ScreenshotID screenshotID, EventIcon icon, EventType type, const std::wstring& destination, const std::wstring& message, const std::wstring& url = L"");
	void EventSetIcon(EventID eventID, EventIcon icon);
	void EventSetProgress(EventID eventID, int pos, int total);
	void EventSetText(EventID eventID, const std::wstring& msg);
	void EventSetURL(EventID eventID, const std::wstring& url);
	void DeleteEvent(EventID eventID);
	void DeleteScreenshot(ScreenshotID screenshotID);

	// IArchiveNotifications events
	void OnPruneScreenshot(ScreenshotID screenshotID);

	// msg handlers
	LRESULT OnDrawItem(DRAWITEMSTRUCT& dis, BOOL& bHandled);
	LRESULT OnMeasureItem(MEASUREITEMSTRUCT& mis, BOOL& bHandled);
	LRESULT OnDeleteItem(DELETEITEMSTRUCT& mis, BOOL& bHandled);

	LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnInternalRegisterScreenshot(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnInternalRegisterEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnReprocess(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClear(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnRemove(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnOpenURL(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnCopyURL(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopyMessage(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnExplore(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnOpenFile(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	void CallInternalRegisterScreenshot(ScreenshotID archiveScreenshotID, Gdiplus::BitmapPtr thumbnail, int screenshotWidth, int screenshotHeight);
	void CallInternalRegisterEvent(EventID archiveEventID, ScreenshotID screenshotID, EventIcon icon, EventType type, const std::wstring& destination, const std::wstring& message, const std::wstring& url = L"");

	void _InternalRegisterScreenshot(ScreenshotID archiveScreenshotID, Gdiplus::BitmapPtr thumbnail, int screenshotWidth, int screenshotHeight);
	void _InternalRegisterEvent(EventID archiveEventID, ScreenshotID screenshotID, EventIcon icon, EventType type, const std::wstring& destination, const std::wstring& message, const std::wstring& url = L"");
	void InternalEventSetIcon(EventID eventID, EventIcon icon);
	void InternalEventSetProgress(EventID eventID, int pos, int total);
	void InternalEventSetText(EventID eventID, const std::wstring& msg);
	void InternalEventSetURL(EventID eventID, const std::wstring& url);
	void InternalDeleteEvent(EventID eventID);
	void InternalDeleteScreenshot(ScreenshotID screenshotID);

	std::list<ActivityListItemSpec> m_items;

	CListBox m_ctl;
	ActivityListItemSpec* m_currentlyInsertingItem;

	ScreenshotArchive& m_archive;
	ScreenshotOptions& m_options;

	// rendering stuff
	ActivityListItemSpec::RenderingInfo m_renderingInfo;

	ActivityListItemSpec* GetFocusedItem() const;
	std::vector<ActivityListItemSpec*> GetSelectedItems() const;
	ActivityListItemSpec* EventIDToItemSpecWithListIndex(EventID id);
	ActivityListItemSpec* GetFirstSelectedItem() const;
	ActivityListItemSpec* GetFirstItemAssociatedWithScreenshot(ScreenshotID screenshotID) const;

	void RedrawItem(ActivityListItemSpec*);
};

#endif

