//
//
//
//
//

#ifndef SCREENIE_ZOOMWND_HPP
#define SCREENIE_ZOOMWND_HPP

#include "animbitmap.h"

class CZoomWindow : public CWindowImpl<CZoomWindow>
{
public:
	DECLARE_WND_CLASS("ScreenieCropperWnd")

  CZoomWindow(util::shared_ptr<Gdiplus::Bitmap> bitmap) :
    m_bitmap(bitmap)  
	{
    m_dibOriginal.SetSize(m_bitmap->GetWidth(), m_bitmap->GetHeight());

    HDC dc = ::GetDC(0);
    HBITMAP hbm = (HBITMAP)SelectObject(dc, bitmap);
    m_dibOriginal.BlitFrom(dc, 0, 0, m_bitmap->GetWidth(), m_bitmap->GetHeight());
    SelectObject(dc, hbm);
    ::ReleaseDC(0, dc);

    m_ImageOrigin.x = 0;
		m_ImageOrigin.y = 0;
	}

	~CZoomWindow() { }

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return TRUE;
	}

	// this sets the would-be cursor position inside the screenshot
	// bitmap. the coordinates are only valid in relation to the bitmap.

	void UpdateBitmapCursorPos(const POINT& origin)
	{
		m_ImageOrigin = origin;
		InvalidateRect(NULL);
	}

	BEGIN_MSG_MAP(CCroppingWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

	LRESULT OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		return 0;
	}

	LRESULT OnSize(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
	  RECT clientRect;
	  GetClientRect(&clientRect);
    if(m_dibOffscreen.GetWidth() < clientRect.right || m_dibOffscreen.GetHeight() < clientRect.bottom)
    {
      m_dibOffscreen.SetSize(clientRect.right, clientRect.bottom);
    }
	  return 0;
	}

	LRESULT OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		PAINTSTRUCT paintStruct = { 0 };
		HDC hdc = BeginPaint(&paintStruct);

		RECT clientRect = { 0 };
		GetClientRect(&clientRect);

    m_dibOffscreen.Fill(0);
    m_dibOriginal.Blit(m_dibOffscreen,
      m_ImageOrigin.x - (clientRect.right / 2),
      m_ImageOrigin.y - (clientRect.bottom / 2),
      clientRect.right,
      clientRect.bottom);
    m_dibOffscreen.Blit(hdc, 0, 0);

    EndPaint(&paintStruct);

		return 0;
	}

	LRESULT OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		return 0;
	}
private:
	POINT m_ImageOrigin;
	util::shared_ptr<Gdiplus::Bitmap> m_bitmap;
  AnimBitmap<32> m_dibOffscreen;
  AnimBitmap<32> m_dibOriginal;
};

#endif