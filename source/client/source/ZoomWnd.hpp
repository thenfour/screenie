//  The name of this class is really not relevant anymore. The ZoomWnd is really the 1:1 window on the left side.
// TODO: make this work in the same way as the other window pane.
//
//
//

#ifndef SCREENIE_ZOOMWND_HPP
#define SCREENIE_ZOOMWND_HPP

#include "animbitmap.h"
#include "image.hpp"



class CZoomWindow :
  public CWindowImpl<CZoomWindow>
{
public:
	DECLARE_WND_CLASS("ScreenieCropperWnd")

  static const int patternFrequency = 8;

  CZoomWindow(Gdiplus::Bitmap* bitmap) :
    m_nFactor(1),
    m_patternOffset(0)
	{
    m_ImageOrigin.x = 0;
    m_ImageOrigin.y = 0;
    m_haveSelection = false;
    CopyImage(m_dibOriginal, *bitmap);
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
    //OutputDebugString(LibCC::Format("UpdateBitmapCursorPos(%,%)|").i<10,3>(origin.x).i<10,3>(origin.y).CStr());
    m_ImageOrigin.x = origin.x;
    m_ImageOrigin.y = origin.y;
    RedrawWindow(0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
  }

  // image coords
	void UpdateBitmapSelectionBox(CRect selection)
	{
    m_haveSelection = true;
    m_selection.left = selection.left;
    m_selection.top = selection.top;
    m_selection.right = selection.right;
    m_selection.bottom = selection.bottom;
    RedrawWindow(0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
  }

	void RemoveSelectionBox()
	{
    m_haveSelection = false;
		InvalidateRect(NULL, FALSE);
  }

	BEGIN_MSG_MAP(CZoomWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

	LRESULT OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		return 0;
	}

  int GetFactor() const
  {
    return m_nFactor;
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
      |_________________.________|   image (scaled to factor)
    */
  CPoint ScaledImageToClient(CPoint image)
  {
    CPoint client;
		RECT clientRect = { 0 };
		GetClientRect(&clientRect);
    int marginx = (m_ImageOrigin.x * m_nFactor) - (clientRect.right / 2);
    int marginy = (m_ImageOrigin.y * m_nFactor) - (clientRect.bottom / 2);
    client.x = (image.x) - marginx;
    client.y = (image.y) - marginy;
    return client;
  }

    /*
      |-----------x return val
             |----x client x coord (argument)
      |-----------------| origin
      |------| margin

             |____x_____.__________| client
      |_________________.________|   image
    */
  CPoint ClientToScaledImage(CPoint client)
  {
    CPoint image;
		RECT clientRect = { 0 };
		GetClientRect(&clientRect);
    int marginx = (m_ImageOrigin.x * m_nFactor) - (clientRect.right / 2);
    int marginy = (m_ImageOrigin.y * m_nFactor) - (clientRect.bottom / 2);
    image.x = (marginx + client.x);
    image.y = (marginy + client.y);
    return image;
  }

  CPoint ScaledImageToImage(CPoint p)
  {
    return CPoint(p.x / m_nFactor, p.y / m_nFactor);
  }

  CPoint ImageToScaledImage(CPoint p)
  {
    return CPoint(p.x * m_nFactor, p.y * m_nFactor);
  }

  CPoint ImageToClient(CPoint image)
  {
    return ScaledImageToClient(ImageToScaledImage(image));
  }

  CPoint ClientToImage(CPoint client)
  {
    return ScaledImageToImage(ClientToScaledImage(client));
  }

	LRESULT OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		PAINTSTRUCT paintStruct = { 0 };
		HDC hdc = BeginPaint(&paintStruct);

		RECT clientRect = { 0 };
		GetClientRect(&clientRect);

    /*
      We can't just assume that the upper left corner is the very start of a pixel (think at high zoom levels.)
      the origin needs to be from the center of the window, since that's where the un-moving crosshair is.

      then from there, just subtract the appropriate pixels in every direction.
    */
    // center client coords.
    CPoint center(clientRect.right / 2, clientRect.bottom / 2);
    // get the # of image pixels we can draw on the screen (given the zoom factor)
    CPoint gridCount(clientRect.right / m_nFactor, clientRect.bottom / m_nFactor);
    gridCount.x /= 2;// because we draw the origin at half the width/height window position
    gridCount.y /= 2;
    // make sure we draw the whole window.
    gridCount.x += 2;
    gridCount.y += 2;
    // subtract the gridcount in order to figure out the UL pixel we will draw.
    CPoint imgUL = m_ImageOrigin;
    imgUL.x -= gridCount.x;
    imgUL.y -= gridCount.y;
    // same for BR
    CPoint imgBR = m_ImageOrigin;
    imgBR.x += gridCount.x;
    imgBR.y += gridCount.y;
    // now get the exact screen coords where we will draw them.
    CPoint clientUL = ImageToClient(imgUL);
    CPoint clientBR = ImageToClient(imgBR);

		CRect rcTemp(0,0,0,0);
		m_dibOffscreen.FillCheckerPattern();
    m_dibOriginal.StretchBlit(m_dibOffscreen,
      clientUL.x, clientUL.y, clientBR.x - clientUL.x, clientBR.y - clientUL.y,
      imgUL.x, imgUL.y, imgBR.x - imgUL.x, imgBR.y - imgUL.y,
      COLORONCOLOR);

    // draw selection rect.
    if(m_haveSelection)
    {
      CRect rcSelection(ImageToClient(m_selection.TopLeft()), ImageToClient(m_selection.BottomRight()));
      //m_dibOffscreen.DrawSelectionRectSafe<patternFrequency, 64, true, false>(m_patternOffset, rcSelection);
    }

    // draw cross-hair
    ICONINFO ii = {0};
    GetIconInfo(LoadCursor(0, IDC_CROSS), &ii);
    DeleteObject(ii.hbmColor);
    DeleteObject(ii.hbmMask);
    ::DrawIcon(m_dibOffscreen.GetDC(),
      (clientRect.right / 2) - ii.xHotspot,
      (clientRect.bottom / 2) - ii.yHotspot,
      LoadCursor(0, IDC_CROSS));

    m_dibOffscreen.Blit(hdc, 0, 0, clientRect.right, clientRect.bottom);

    EndPaint(&paintStruct);

		return 0;
	}

	LRESULT OnDestroy(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		return 0;
	}

  void SetFactor(int n)
  {
    if(m_nFactor != n)
    {
      m_nFactor = n;
      RedrawWindow(0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
    }
  }

private:
	POINT m_ImageOrigin;// origin, in m_dibOriginal coords
  bool m_haveSelection;
  CRect m_selection;// selection box, in m_dibOriginal coords
  AnimBitmap<32> m_dibOffscreen;

  int m_patternOffset;

  int m_nFactor;
  AnimBitmap<32> m_dibOriginal;
};

#endif

