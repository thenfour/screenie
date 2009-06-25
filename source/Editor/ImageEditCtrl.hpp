// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark

#ifndef SCREENIE_IMAGEEDITWND_HPP
#define SCREENIE_IMAGEEDITWND_HPP

#include "ToolBase.h"
#include <map>
#include <stack>
#include "resource.h"
#include "Viewport.h"
#include "Viewport.h"


inline Gdiplus::RectF ToRectF(const RECT& rc)
{
	return Gdiplus::RectF((Gdiplus::REAL)rc.left, (Gdiplus::REAL)rc.top, (Gdiplus::REAL)(rc.right - rc.left), (Gdiplus::REAL)(rc.bottom - rc.top));
}


inline void DumpBitmap(Gdiplus::Bitmap& image, int x, int y)
{
  // draw that damn bitmap to the screen.
  HDC dc = ::GetDC(0);
  HDC dcc = CreateCompatibleDC(dc);
  HBITMAP himg;
  image.GetHBITMAP(0, &himg);
  HBITMAP hOld = (HBITMAP)SelectObject(dcc, himg);
  StretchBlt(dc, x, y, image.GetWidth(), image.GetHeight(), dcc, 0, 0, image.GetWidth(), image.GetHeight(), SRCCOPY);
  SelectObject(dcc, hOld);
  DeleteDC(dcc);
  DeleteObject(himg);
  ::ReleaseDC(0,dc);
}

inline std::wstring SizeToString(const Gdiplus::SizeF& s)
{
	return LibCC::Format(L"%,% (area:%)")
		.d(s.Width, 1, 3, ' ')
		.d(s.Height, 1, 3, ' ')
		.d(s.Width * s.Height, 1, 3, ' ')
		.Str();
}


/////////////////////////////////////////////////////////////////////////////////////////
class IImageEditWindowEvents
{
public:
  virtual void OnSelectionChanged() = 0;
	virtual void OnCursorPositionChanged(const Gdiplus::PointF&) = 0;
  virtual void OnZoomFactorChanged() = 0;
	virtual void OnToolChanging(ToolBase* tool) = 0;
};


/////////////////////////////////////////////////////////////////////////////////////////
class ImageSurface
{
public:

	bool IsNull() const
	{
		if(0 == m_bitmap.get())
			return true;
		if(0 == m_invalidRegion.get())
			return true;
		if(0 == m_g.get())
			return true;
		return false;
	}

	void EnsureBigEnough(const Gdiplus::SizeF& s)
	{
		if(0 != m_bitmap.get())
		{
			if(m_bitmap->GetWidth() >= s.Width)
			{
				if(m_bitmap->GetHeight() >= s.Height)
					return;// it's OK.
			}
		}

		// requires recreating.
		m_bitmap.reset(new Gdiplus::Bitmap((UINT)s.Width, (UINT)s.Height, PixelFormat24bppRGB));
		m_invalidRegion.reset(new Gdiplus::Region(Gdiplus::RectF(0, 0, s.Width, s.Height)));
		m_g.reset(new Gdiplus::Graphics(m_bitmap.get()));
		m_g->ResetTransform();
		return;
	}

	void Reset(Gdiplus::Bitmap& source)
	{
		Gdiplus::SizeF s;
		source.GetPhysicalDimension(&s);
		m_bitmap.reset(source.Clone(0, 0, (UINT)s.Width, (UINT)s.Height, PixelFormat24bppRGB));
		m_invalidRegion.reset(new Gdiplus::Region(Gdiplus::RectF(0, 0, s.Width, s.Height)));
		m_g.reset(new Gdiplus::Graphics(m_bitmap.get()));
		m_g->ResetTransform();
	}

	Gdiplus::Graphics& Graphics()
	{
		return *m_g.get();
	}

	Gdiplus::Bitmap& Bitmap()
	{
		return *m_bitmap.get();
	}

	void Invalidate(const Gdiplus::RectF& rc)
	{
		m_invalidRegion->Union(rc);
	}

	void Invalidate()
	{
		m_invalidRegion->MakeEmpty();
		Gdiplus::SizeF s = GetSize();
		m_invalidRegion->Union(Gdiplus::RectF(0, 0, s.Width, s.Height));
	}

	void Validate(const Gdiplus::RectF& rc)
	{
		m_invalidRegion->Exclude(rc);
	}

	bool IsInvalid(const Gdiplus::RectF& rc)
	{
		return TRUE == m_invalidRegion->IsVisible(rc);
	}

	// gets the rectangle that is WITHIN "working", which contains m_invalidRegion.
	Gdiplus::RectF GetUpdateRect(Gdiplus::RectF& working) const
	{
		std::auto_ptr<Gdiplus::Region> temp(m_invalidRegion->Clone());
		temp->Intersect(working);
		Gdiplus::RectF ret;
		temp->GetBounds(&ret, m_g.get());
		return ret;
	}

	Gdiplus::SizeF GetSize() const
	{
		Gdiplus::SizeF ret;
		m_bitmap->GetPhysicalDimension(&ret);
		return ret;
	}

private:
	std::auto_ptr<Gdiplus::Bitmap> m_bitmap;
	std::auto_ptr<Gdiplus::Region> m_invalidRegion;
	std::auto_ptr<Gdiplus::Graphics> m_g;
};


/////////////////////////////////////////////////////////////////////////////////////////
class CImageEditWindow :
  public CWindowImpl<CImageEditWindow>
{
  // global const settings
  static const int patternFrequency = 8;

public:
  // ATL shit
  static ATL::CWndClassInfo& GetWndClassInfo();

protected:

	BEGIN_MSG_MAP(CImageEditWindow)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_RBUTTONUP, OnRButtonUp)
    MESSAGE_HANDLER(WM_CAPTURECHANGED, OnLoseCapture)
    MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
    MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)

		COMMAND_HANDLER(ID_EDIT_COPY, 0, OnCopy)
		COMMAND_HANDLER(ID_EDIT_PASTE, 0, OnPaste)
  END_MSG_MAP()

	CImageEditWindow() :
		m_isLeftDragging(false),
		m_isRightDragging(false),
		m_dragRefCount(0)
	{
	}

	void SetDocument(Gdiplus::Bitmap& src)
	{
		m_document.Reset(src);
		m_viewport.SetZoomFactor(0.8f);
	}

	LRESULT OnCommand(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		return 0;
	}

	LRESULT OnContextMenu(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		return 0;
	}

	LRESULT OnCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		return 0;
	}

	LRESULT OnSize(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		return 0;
	}

	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
	  POINTS& psTemp = MAKEPOINTS(lParam);
		Gdiplus::PointF ptView((Gdiplus::REAL)psTemp.x, (Gdiplus::REAL)psTemp.y);
		Gdiplus::PointF ptDocument(m_viewport.ViewToDocument(ptView));

		// handle panning movement.
		if(m_isRightDragging)
		{
			Gdiplus::PointF distancePannedView = ptView - m_lastKnownViewCursor;
			Gdiplus::REAL maybeZero = distancePannedView.X + distancePannedView.Y;
			if(abs(maybeZero) > 0.001f)
			{
				m_viewport.SetViewOrigin(m_viewport.GetViewOrigin() + distancePannedView);
				m_offscreen.Invalidate();
				Invalidate(FALSE);
			}
		}

		m_lastKnownViewCursor = ptView;
		m_lastKnownDocumentCursor = ptDocument;

		return 0;
	}

	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		_SetCapture();
		m_isLeftDragging = true;
		return 0;
	}

	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		_ReleaseCapture();
		m_isLeftDragging = false;
		return 0;
	}

	LRESULT OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		_SetCapture();
		m_isRightDragging = true;
		return 0;
	}

	LRESULT OnRButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		_ReleaseCapture();
		m_isRightDragging = false;
		return 0;
	}

	LRESULT OnLoseCapture(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_isLeftDragging = false;
		m_isRightDragging = true;
		return 0;
	}

	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnPaste(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnCopy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& handled)
	{
		// {{{ DEBUGGING
		LibCC::LogScopeMessage l("CImageWindow::OnPaint");
		LibCC::Timer t;
		LibCC::Timer t2;
		t.Tick();
		// }}}

		PAINTSTRUCT ps;
		BeginPaint(&ps);
		Gdiplus::RectF viewUpdate = ToRectF(ps.rcPaint);
		Gdiplus::RectF documentUpdate = m_viewport.ViewToDocument(viewUpdate);

		// {{{ DEBUGGING
		RECT rcClient1;
		GetClientRect(&rcClient1);
		Gdiplus::RectF rcClient2 = ToRectF(rcClient1);
		Gdiplus::SizeF clientSize;
		rcClient2.GetSize(&clientSize);
		//LibCC::g_pLog->Message(LibCC::Format(L"window size              : %")(SizeToString(clientSize)));
		int totalPixelsRendered = 0;
		// }}}

		if(m_document.IsNull())
		{
			EndPaint(&ps);
			return 0;
		}

		// make sure the surfaces are set up.
		m_flatDocument.EnsureBigEnough(m_document.GetSize());
		RECT rcClient;
		GetClientRect(&rcClient);
		m_offscreen.EnsureBigEnough(Gdiplus::SizeF((Gdiplus::REAL)rcClient.right, (Gdiplus::REAL)rcClient.bottom));// remember rcClient left/top are always 0.

		if(m_flatDocument.IsInvalid(documentUpdate))
		{
			t2.Tick();

			Gdiplus::RectF updateArea(m_flatDocument.GetUpdateRect(documentUpdate));

			m_flatDocument.Graphics().DrawImage(&m_document.Bitmap(),
				updateArea,
				updateArea.X, updateArea.Y,
				updateArea.Width, updateArea.Height, Gdiplus::UnitPixel);

			// TODO: call tools

			// {{{ DEBUGGING
			totalPixelsRendered += (int)(updateArea.Width * updateArea.Height);
			Gdiplus::SizeF temp;
			m_viewport.DocumentToView(updateArea).GetSize(&temp);
			//LibCC::g_pLog->Message(LibCC::Format(L"flatdoc dirty render size: %")(SizeToString(temp)));
			// }}}

			m_flatDocument.Validate(updateArea);
			m_offscreen.Invalidate(m_viewport.DocumentToView(updateArea));

			t2.Tick();
			//LibCC::g_pLog->Message(LibCC::Format(L"Rendered flatdoc in % seconds").d<4>(t2.GetLastDelta()));
		}

		if(m_offscreen.IsInvalid(viewUpdate))
		{
			t2.Tick();

			Gdiplus::RectF updateAreaView(m_offscreen.GetUpdateRect(viewUpdate));
			Gdiplus::RectF updateAreaDoc(m_viewport.ViewToDocument(updateAreaView));

			// draw the checkered background (TODO)
			Gdiplus::SolidBrush brush(Gdiplus::Color(128,128,128));
			m_offscreen.Graphics().FillRectangle(&brush, updateAreaView);

			// draw the zoomed document here.
			m_offscreen.Graphics().SetInterpolationMode(
				m_viewport.GetZoomFactor() >= 1.0 ? Gdiplus::InterpolationModeNearestNeighbor : Gdiplus::InterpolationModeBilinear);

			m_offscreen.Graphics().DrawImage(&m_flatDocument.Bitmap(),
				updateAreaView,
				updateAreaDoc.X, updateAreaDoc.Y,
				updateAreaDoc.Width,
				updateAreaDoc.Height,
				Gdiplus::UnitPixel);

			// {{{ DEBUGGING
			totalPixelsRendered += (int)(updateAreaDoc.Width * updateAreaDoc.Height);
			Gdiplus::SizeF temp;
			updateAreaView.GetSize(&temp);
			//LibCC::g_pLog->Message(LibCC::Format(L"offscreen render size    : %")(SizeToString(temp)));
			// }}}

			// TODO: call tools

			m_offscreen.Validate(updateAreaView);

			t2.Tick();
			//LibCC::g_pLog->Message(LibCC::Format(L"Rendered offscreen in % seconds").d<4>(t2.GetLastDelta()));
		}

		{
			//DumpBitmap(m_flatDocument.Bitmap(), 0, 0);
			//Gdiplus::Graphics g((HWND)*this);
			//g.DrawImage(&m_document.Bitmap(), 0, 0);
		}

		{
			Gdiplus::Graphics g(ps.hdc);
			Gdiplus::RectF rcPaintF = ToRectF(ps.rcPaint);
			g.DrawImage(&m_offscreen.Bitmap(),
				rcPaintF,
				rcPaintF.X, rcPaintF.Y,
				rcPaintF.Width, rcPaintF.Height,
				Gdiplus::UnitPixel
				);


			// {{{ DEBUGGING
			totalPixelsRendered += (int)(rcPaintF.Width * rcPaintF.Height);
			Gdiplus::SizeF temp;
			rcPaintF.GetSize(&temp);
			//LibCC::g_pLog->Message(LibCC::Format(L"to screen              : %")(SizeToString(temp)));
			//LibCC::g_pLog->Message(LibCC::Format(L"TOTAL PIXELS           : %")(totalPixelsRendered));
			// }}}
		}

		EndPaint(&ps);

		//LibCC::g_pLog->Message(LibCC::Format(L"TOTAL PIXELS           : %")(totalPixelsRendered));
		t.Tick();
		LibCC::g_pLog->Message(LibCC::Format(L"Rendered in % seconds").d<4>(t.GetLastDelta()));

		return 0;
	}

private:

	void _SetCapture()
	{
		if(m_dragRefCount == 0)
		{
			SetCapture();
		}
		m_dragRefCount ++;
	}
	void _ReleaseCapture()
	{
		_ASSERT(m_dragRefCount != 0);
		m_dragRefCount --;
		if(0 == m_dragRefCount)
			ReleaseCapture();
	}

	int m_dragRefCount;// because you can be dragging the left OR right buttons, i need a ref count to know when to releasecapture.
	bool m_isRightDragging;
	bool m_isLeftDragging;

	Gdiplus::PointF m_lastKnownViewCursor;
	Gdiplus::PointF m_lastKnownDocumentCursor;


	ImageSurface m_document;
	ImageSurface m_flatDocument;// document, but with temporary stuff drawn on top that can be undone easily (like highlighter while drawing)
	Viewport m_viewport;
	ImageSurface m_offscreen;
};

#endif

