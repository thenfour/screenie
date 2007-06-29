
#ifndef SCREENIE_ACTIVITYLISTITEM_HPP
#define SCREENIE_ACTIVITYLISTITEM_HPP

#include "ScreenshotArchive.hpp"
#include "ProgressImages.hpp"
#include "AnimBitmap.h"

class ActivityListItemSpec// attached to each item of the listbox
{
public:
	enum ItemType
	{
		Screenshot,
		Event
	};
	ActivityListItemSpec();
	~ActivityListItemSpec();

	struct RenderingInfo
	{
		RenderingInfo();

		HWND m_hwnd;
		int m_lineHeight;
		int m_windowWidth;

		int ScreenshotThumbBorderSize;
		CRect ScreenshotPadding;
		int ScreenshotCaptionMarginLeft;
		int ScreenshotSeparatorHeight;
		CRect EventPadding;
		int EventIconPaddingRight;

		CImageList m_imageList;
		int m_iconHeight;
		int m_iconWidth;
		ProgressImages m_progress;
		int m_iconInfo;
		int m_iconWarning;
		int m_iconError;
		int m_iconCheck;
		int EventIconToIconIndex(const EventIcon& t)
		{
			switch(t)
			{
			case EI_INFO:
				return m_iconInfo;
			case EI_WARNING:
				return m_iconWarning;
			case EI_ERROR:
				return m_iconError;
			case EI_CHECK:
				return m_iconCheck;
			case EI_PROGRESS:
				return m_progress.GetImageFromProgress(0,1);// just return 0%
			}
			return m_iconError;
		}
	};

	void Init(RenderingInfo* p) { m_info = p; }

	ItemType GetType() const { return m_type; }
	void SetType(ItemType n) { _SetValue(m_type, n); }

	EventID GetEventID() const { return m_eventID; }
	void SetEventID(EventID id) { _SetValue(m_eventID, id); }

	EventIcon GetEventIcon() const { return m_eventIcon; }
	void SetEventIcon(EventIcon v) { _SetValue(m_eventIcon, v); }

	EventType GetEventType() const { return m_eventType; }
	void SetEventType(EventType v) { _SetValue(m_eventType, v); }

	std::wstring GetEventDestination() const { return m_eventDestination; }
	void SetEventDestination(const std::wstring& v) { _SetValue(m_eventDestination, v); }

	std::wstring GetEventMessage() const { return m_eventMessage; }
	void SetEventMessage(const std::wstring& v) { _SetValue(m_eventMessage, v); }

	std::wstring GetEventURL() const { return m_eventURL; }
	void SetEventURL(const std::wstring& v) { _SetValue(m_eventURL, v); }

	void SetEventImageIndex(int v) { _SetValue(m_eventImageIndex, v); }

	ScreenshotID GetScreenshotID() const { return m_screenshotID; }
	void SetScreenshotID(ScreenshotID v)
	{
		_SetValue(m_screenshotID, v);
	}

	int GetScreenshotHeight() const { return m_screenshotHeight; }
	void SetScreenshotHeight(int v) { _SetValue(m_screenshotHeight, v); }

	int GetScreenshotWidth() const { return m_screenshotWidth; }
	void SetScreenshotWidth(int v) { _SetValue(m_screenshotWidth, v); }

	void SetDate(const std::wstring& v) { _SetValue(m_date, v); }

	int GetListIndex() const { return m_listIndex; }
	void SetListIndex(int v) { _SetValue(m_listIndex, v); }

	void SetThumbnail(Gdiplus::BitmapPtr thumbnail);

	int GetItemHeight();
	bool RenderTo(HDC dc, CRect rc, bool selected);

private:
	// helper to set a value & set dirty or not.
	template<typename T>
	void _SetValue(T& dest, const T& src)
	{
		if(dest == src) return;
		dest = src;
		m_dirty = true;
	}
	void _SetValue(ScreenshotID& dest, const ScreenshotID& src)
	{
		//_SetValue<ScreenshotID>(dest, src);  <-- WTF this gives a compiler error!!!!
		if(dest == src) return;
		dest = src;
		m_dirty = true;
	}
	void ReleaseThumbnail();

	ItemType m_type;


	// for events
	EventID m_eventID;
  EventType m_eventType;
	EventIcon m_eventIcon;
	std::wstring m_eventDestination;
	std::wstring m_eventMessage;
	std::wstring m_eventURL;
	int m_eventImageIndex;

	// for screenshot
	int m_screenshotHeight;
	int m_screenshotWidth;

	// for all
	ScreenshotID m_screenshotID;
	std::wstring m_date;

	// only valid sometimes...
	int m_listIndex;

	// For rendering
	RenderingInfo* m_info;

	void RenderOffscreen();
	void CalculateDimensions();
	bool m_selectedState;

	bool m_dirty;

	int m_itemWidth;
	int m_itemHeight;

	std::tr1::shared_ptr<Gdiplus::Bitmap> m_thumb;

	CDC m_offscreen;
	CBitmap m_offscreenbm;
	//std::tr1::shared_ptr<Gdiplus::Bitmap> m_offscreen;
	//std::tr1::shared_ptr<Gdiplus::Graphics> m_offscreenGfx;
	//HBITMAP m_hbmThumb;
	//HBITMAP m_oldThumbBitmap;
	//HDC m_dcThumb;
	//int m_thumbWidth;
	//int m_thumbHeight;
};
#endif

