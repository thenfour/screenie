

#pragma once


// this helps with all bitmap displaying / caching / etc used by the image edit window.
// this way, the image window can handle mechanics of GUI, and all the muck of 
// caching, calculating coordinates, etc of drawing the bitmaps can be stuck here.
class ImageEditRenderer
{
public:
	ImageEditRenderer() :
		m_original(0),
		clientWidth(0),
		clientHeight(0),
		hwnd(0),
		m_hasSelection(false)
	{
	}
	const Viewport& GetViewport() const
	{
		return m_view;
	}

	void SetHWND(HWND h)
	{
		hwnd = h;
		RECT rc;
		GetClientRect(hwnd, &rc);
		clientWidth = rc.right;
		clientHeight = rc.bottom;
	}
	void SetOriginalImage(AnimBitmap<32>& o)
	{
		m_original = &o;
	}
	void SetZoomFactor(ViewPortSubPixel z)
	{
		m_view.SetZoomFactor(z);
	}
	void ClearSelection()
	{
		m_hasSelection = false;
	}
	void SetSelectionRect(const CRect& imgCoords)
	{
		m_hasSelection = true;
		m_selectionImg = imgCoords;
	}
	bool HasSelection() const  { return m_hasSelection; }
	CRect GetSelection() const { return m_selectionImg; }
  void SetViewOrigin(const PointF& o)
	{
		m_view.SetViewOrigin(o);
	}
  void SetImageOrigin(const PointF& o)
	{
		m_view.SetImageOrigin(o);
	}
	void SetClientSize(int width, int height)
	{
		clientWidth = width;
		clientHeight = height;
	}

	void Render(HDC dc)
	{
		if(m_dibOffscreen.GetWidth() < clientWidth || m_dibOffscreen.GetHeight() < clientHeight)
		{
			m_dibOffscreen.SetSize(clientWidth, clientHeight);
			m_dibStretched.SetSize(clientWidth, clientHeight);
		}

		// figure out an integral place to start drawing (client 0,0 may be at a fraction of a virtual coord)
		PointF ulVirtual = m_view.ViewToImage(PointF(0, 0));
		ulVirtual.x = floor(ulVirtual.x);// make integral
		ulVirtual.y = floor(ulVirtual.y);// make integral
		PointF ulClient = m_view.ImageToView(ulVirtual);// now this is guaranteed to be at the start of an integral virtual coord

		// do the similar stuff for bottom-right
		PointF brVirtual = m_view.ViewToImage(PointF((ViewPortSubPixel)clientWidth, (ViewPortSubPixel)clientHeight));
		brVirtual.x = ceil(brVirtual.x);// make integral
		brVirtual.y = ceil(brVirtual.y);// make integral
		PointF brClient = m_view.ImageToView(brVirtual);// now this is guaranteed to be at the start of an integral virtual coord

		// figure out the dimensions of the entire original image to draw
		PointF viewSize(brClient.x - ulClient.x, brClient.y - ulClient.y);
		PointF virtualSize(brVirtual.x - ulVirtual.x, brVirtual.y - ulVirtual.y);

		//OutputDebugString(LibCC::Format("(%,%)-(%,%)|")(excl.left)(excl.top)(excl.right)(excl.bottom).CStr());

		m_original->StretchBlit(m_dibStretched,
			long(ulClient.x),// destx
			long(ulClient.y),// desty
			long(viewSize.x),// destw
			long(viewSize.y),// desth
			long(ulVirtual.x),// srcx
			long(ulVirtual.y),// srcy
			long(virtualSize.x),// srcw
			long(virtualSize.y),// srcy
			(m_view.GetZoomFactor() < 1.0) ? HALFTONE : COLORONCOLOR
			);

		m_dibStretched.Blit(m_dibOffscreen, 0, 0, clientWidth, clientHeight);

		if(m_hasSelection)
		{
			int patternOffset = 0;
			PointF ul(m_selectionImg.left, m_selectionImg.top);
			PointF br(m_selectionImg.right, m_selectionImg.bottom);
			ul = m_view.ImageToView(ul);
			br = m_view.ImageToView(br);
			CRect rcSelection(ul.Round(), br.Round());
			m_dibOffscreen.DrawSelectionRectSafe<8, 64, true, false>(patternOffset, rcSelection, AnimBitmap<32>::SR_DEFAULT);
		}

		m_dibOffscreen.Blit(dc, 0, 0);
	}

private:
	AnimBitmap<32>* m_original;
	AnimBitmap<32> m_dibOffscreen;
	AnimBitmap<32> m_dibStretched;
  Viewport m_view;
	int clientWidth;
	int clientHeight;
	HWND hwnd;

	bool m_hasSelection;
	CRect m_selectionImg;

	// cache bitmaps
	void CacheZoomed()
	{
		// generate the m_dibZoomed
		// and the grayed-out version
	}

	AnimBitmap<32> m_dibZoomed;// cache of zoomed bitmap.
	CPoint m_zoomedOrigin;// image coords of the origin of m_dibZoomed (the upper-left corner of it)
};


