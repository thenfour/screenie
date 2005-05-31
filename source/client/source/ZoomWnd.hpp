//
//
//
//
//

#ifndef SCREENIE_ZOOMWND_HPP
#define SCREENIE_ZOOMWND_HPP

#include "animbitmap.h"
#include "image.hpp"

class CZoomWindow : public CWindowImpl<CZoomWindow>
{
public:
	DECLARE_WND_CLASS("ScreenieCropperWnd")

  static const int ZoomFactor = 2;

  CZoomWindow(Gdiplus::Bitmap* bitmap)
	{
    m_haveSelection = false;
    CopyImage(m_dibOriginalZoomed, *bitmap, ZoomFactor);
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
    m_ImageOrigin.x = origin.x * ZoomFactor;
    m_ImageOrigin.y = origin.y * ZoomFactor;
		InvalidateRect(NULL);
  }

	void UpdateBitmapSelectionBox(CRect selection)
	{
    m_haveSelection = true;
    m_selection.left = selection.left * ZoomFactor;
    m_selection.top = selection.top * ZoomFactor;
    m_selection.right = selection.right * ZoomFactor;
    m_selection.bottom = selection.bottom * ZoomFactor;
		InvalidateRect(NULL);
  }

	void RemoveSelectionBox()
	{
    m_haveSelection = false;
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

    /*
             |------x  return val
      |-------------x image coord (argument)
      |-----------------| origin
      |------| margin

             |__________.__________| client
      |___x______________________|   image
    */
  CPoint ImageToClient(CPoint image)
  {
    CPoint client;
		RECT clientRect = { 0 };
		GetClientRect(&clientRect);
    int marginx = m_ImageOrigin.x - (clientRect.right / 2);
    int marginy = m_ImageOrigin.y - (clientRect.bottom / 2);
    client.x = image.x - marginx;
    client.y = image.y - marginy;
    return client;
  }

    /*
      |-----------x return val
             |----x client x coord (argument)
      |-----------------| origin
      |------| margin

             |____x_____.__________| client
      |__________________________|   image
    */
  CPoint ClientToImage(CPoint client)
  {
    CPoint image;
		RECT clientRect = { 0 };
		GetClientRect(&clientRect);
    int marginx = m_ImageOrigin.x - (clientRect.right / 2);
    int marginy = m_ImageOrigin.y - (clientRect.bottom / 2);
    image.x = marginx + client.x;
    image.y = marginy + client.y;
    return image;
  }

	LRESULT OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		PAINTSTRUCT paintStruct = { 0 };
		HDC hdc = BeginPaint(&paintStruct);

		RECT clientRect = { 0 };
		GetClientRect(&clientRect);

    CPoint img = ClientToImage(0);

    m_dibOffscreen.Fill(0);
    m_dibOriginalZoomed.Blit(m_dibOffscreen,
      img.x, img.y,
      clientRect.right,
      clientRect.bottom);

    // draw selection rect.
    if(m_haveSelection)
    {
      CRect rcSelection(ImageToClient(m_selection.TopLeft()), ImageToClient(m_selection.BottomRight()));
      DrawFocusRect(m_dibOffscreen.GetDC(), &rcSelection);
    }

    // draw cross-hair
    ICONINFO ii = {0};
    GetIconInfo(LoadCursor(0, IDC_CROSS), &ii);
    ::DrawIcon(m_dibOffscreen.GetDC(),
      (clientRect.right / 2) - ii.xHotspot,
      (clientRect.bottom / 2) - ii.yHotspot,
      LoadCursor(0, IDC_CROSS));

    m_dibOffscreen.Blit(hdc, 0, 0);

    EndPaint(&paintStruct);

		return 0;
	}

	LRESULT OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		return 0;
	}
private:
	POINT m_ImageOrigin;// origin, in m_dibOriginal coords
  bool m_haveSelection;
  CRect m_selection;// selection box, in m_dibOriginal coords
  AnimBitmap<32> m_dibOffscreen;
  AnimBitmap<32> m_dibOriginalZoomed;
};

#endif