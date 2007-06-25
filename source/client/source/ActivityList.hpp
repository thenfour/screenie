/*
Where do messages go?

(destination.cpp) -> statusdlg -> activitylist
                               -> archive

When do messages show up?
when you first open it up, it shows all messages in the archive, period. thumbnails are cached in the database.


*/

#ifndef SCREENIE_ACTIVITYLIST_HPP
#define SCREENIE_ACTIVITYLIST_HPP

#include "animbitmap.h"
#include "polarlut.h"
#include "ScreenshotArchive.hpp"


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
	public IActivity
{
	CListViewCtrl m_ctl;
public:
	ActivityList() { }

	BEGIN_MSG_MAP(ActivityList)
	END_MSG_MAP()

	void Attach(HWND h)
	{
		m_ctl = h;
		SubclassWindow(h);
	}

	virtual ~ActivityList() { }

	ScreenshotID RegisterScreenshot(Gdiplus::BitmapPtr image){ return 0;}
	EventID RegisterEvent(ScreenshotID screenshotID, EventIcon icon, EventType type, const std::wstring& destination, const std::wstring& message, const std::wstring& url = L""){return 0;}
	void EventSetIcon(EventID eventID, EventIcon icon){}
	void EventSetProgress(EventID eventID, int pos, int total){}
	void EventSetText(EventID eventID, const std::wstring& msg){}
	void EventSetURL(EventID eventID, const std::wstring& url){}

private:
	struct ItemSpec// attached to each item of a listview
	{
		enum
		{
			Screenshot,
			Event
		}
		type;

		// for screenshots
		Gdiplus::BitmapPtr thumb;

		// for events
    EventType eventType;
		std::wstring msg;
		std::wstring url;

		// for all
		std::wstring date;
	};

	ItemSpec* EventIDToItemSpec(EventID msgID);// returns 0 if not found.
};

#endif

