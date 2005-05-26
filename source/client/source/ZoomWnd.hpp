//
//
//
//
//

#ifndef SCREENIE_ZOOMWND_HPP
#define SCREENIE_ZOOMWND_HPP

class CZoomWindow : public CWindowImpl<CZoomWindow>
{
public:
	DECLARE_WND_CLASS("ScreenieCropperWnd")

	CZoomWindow(util::shared_ptr<Gdiplus::Bitmap> bitmap) : m_bitmap(bitmap)
	{
		m_cursorPos.x = 0;
		m_cursorPos.y = 0;
	}

	~CZoomWindow() { }

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return TRUE;
	}

	// this sets the would-be cursor position inside the screenshot
	// bitmap. the coordinates are only valid in relation to the bitmap.

	void UpdateBitmapCursorPos(const POINT& cursorPos)
	{
		m_cursorPos = cursorPos;
		InvalidateRect(NULL);
	}

	BEGIN_MSG_MAP(CCroppingWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()

	LRESULT OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		return 0;
	}

	LRESULT OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		PAINTSTRUCT paintStruct = { 0 };
		HDC hdc = BeginPaint(&paintStruct);

		RECT clientRect = { 0 };
		GetClientRect(&clientRect);

		Gdiplus::Graphics windowGraphics(hdc);

		windowGraphics.Clear(Gdiplus::Color(0, 0, 0));
		windowGraphics.DrawImage(m_bitmap.get(), 0, 0,
			m_cursorPos.x - (clientRect.right / 4),
			m_cursorPos.y - (clientRect.bottom / 4),
			clientRect.right, clientRect.bottom,
			Gdiplus::UnitPixel);

		EndPaint(&paintStruct);

		return 0;
	}

	LRESULT OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		return 0;
	}
private:
	POINT m_cursorPos;
	util::shared_ptr<Gdiplus::Bitmap> m_bitmap;
};

#endif