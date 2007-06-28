/*
Where do messages go?

(destination.cpp) -> statusdlg -> activitylist -> archive

It's simplest to forward messages like this. the ONLY time that the reverse flow needs to happen
is when the archive self-cleans, and needs to tell the activitylist to remove something. No problem,
that's what IArchiveNotifications is for.

*/

#ifndef SCREENIE_ACTIVITYLIST_HPP
#define SCREENIE_ACTIVITYLIST_HPP

#include "animbitmap.h"
#include "resource.h"
#include "polarlut.h"
#include "ScreenshotArchive.hpp"
#include <list>

class ProgressImages
{
public:
  void InitializeProgressImages(CImageList& img, RgbPixel32 background, RgbPixel32 filled, RgbPixel32 unfilled);
  int GetImageFromProgress(int pos, int total);
private:
  /*
    this will hold linear from 0-perimiter.
    assume that the image list indices will not change.
  */
  std::vector<int> m_images;
  AngleLut<float> m_angles;

  void DrawHLine(long x1, long x2, long y);
  void DrawAlphaPixel(long cx, long cy, long x, long y, long f, long fmax);
  RgbPixel32 PositionToColor(long x, long y);

  // stuff that we persist between drawing calls
  AnimBitmap<32>* m_bmp;
  int m_i;

  float m_pieBlurringSize;
  int m_perimeter;
  int m_diameter;
  int m_radius;
  RgbPixel32 m_background;
  RgbPixel32 m_unfilled;
  RgbPixel32 m_filled;
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

	ActivityList(ScreenshotArchive& archive) :
		m_archive(archive),
		m_currentlyInsertingItem(0),
		m_itemHeight(0)
	{
	}

	BEGIN_MSG_MAP(ActivityList)
    COMMAND_ID_HANDLER(IDC_CLEAR, OnClear)
    COMMAND_ID_HANDLER(IDC_REMOVE, OnRemove)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)

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

	LRESULT OnReprocess(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnClear(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnRemove(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnOpenURL(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnCopyURL(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopyMessage(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnExplore(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnOpenFile(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	void InternalRegisterScreenshot(ScreenshotID archiveScreenshotID, Gdiplus::BitmapPtr thumbnail);
	void InternalRegisterEvent(EventID archiveEventID, ScreenshotID screenshotID, EventIcon icon, EventType type, const std::wstring& destination, const std::wstring& message, const std::wstring& url = L"");
	void InternalEventSetIcon(EventID eventID, EventIcon icon);
	void InternalEventSetProgress(EventID eventID, int pos, int total);
	void InternalEventSetText(EventID eventID, const std::wstring& msg);
	void InternalEventSetURL(EventID eventID, const std::wstring& url);
	void InternalDeleteEvent(EventID eventID);
	void InternalDeleteScreenshot(ScreenshotID screenshotID);

	struct ItemSpec// attached to each item of a listview
	{
		enum
		{
			Screenshot,
			Event
		}
		type;

		// for screenshots
		ScreenshotID screenshotID;
		Gdiplus::BitmapPtr thumb;

		// for events
		EventID eventID;
    EventType eventType;
		EventIcon icon;
		std::wstring destination;
		std::wstring msg;
		std::wstring url;

		// for all
		std::wstring date;

		// only valid sometimes...
		int listIndex;
	};
	std::list<ItemSpec> m_items;


	//CListViewCtrl m_ctl;
	CListBox m_ctl;
	ItemSpec* m_currentlyInsertingItem;

	ScreenshotArchive& m_archive;

	// rendering stuff
	CBrush m_brushNormal;
	CBrush m_brushSelected;
	int m_itemHeight;

	ItemSpec* GetFocusedItem() const;
	std::vector<ItemSpec*> GetSelectedItems() const;
	ItemSpec* EventIDToItemSpec(EventID msgID);// returns 0 if not found.
	ItemSpec* ScreenshotIDToItemSpec(ScreenshotID screenshotID);
};

#endif

