// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark

/*
	Tools should get an offscreen area where they can draw temporary stuff on their PaintClient callback. It should be the original size, not zoomed.
	So, here's how rendering works.

	ImageEditWnd::m_bitmap is the original screenshot. A backup. It's only copied rarely.
	ImageEditWnd::m_dibOriginal is the copied screenshot. This is the "canvas" - the actual document which will be copied from.
	ImageEditWnd::m_offscreen is the copied screenshot plus any temporary tool paintings. When the tool declares it has temporary rendering, they can draw on this. Each paint, it's copied from m_dibOriginal, so it's always wiped clean.
	then it hits ImageEditRenderer and this "original image canvas" gets copied to a Zoomed canvas with just the zoomed image on it
	It's also rendered as a zoomed-grayed image
	And then offscreen is a mixture of these based on selection rect
	And it's rendered to the window.

*/


#pragma once

#include "image.hpp"

template<typename T>
std::wstring RectToString(const T& rc)
{
	return LibCC::FormatW(L"(%,%)-(%,%) / w:% / h:%")
		(rc.left)(rc.top)(rc.right)(rc.bottom)
		(rc.right - rc.left)
		(rc.bottom - rc.top)
		.Str();
}

template<typename T>
std::wstring PointToString(const T& pt)
{
	return LibCC::FormatW(L"(%,%)")
		(pt.x)(pt.y)
		.Str();
}

template<typename T>
std::wstring SizeToString(const T& pt)
{
	return LibCC::FormatW(L"(%,%)")
		(pt.cx)(pt.cy)
		.Str();
}


// this helps with all bitmap displaying / caching / etc used by the image edit window.
// this way, the image window can handle mechanics of GUI, and all the muck of 
// caching, calculating coordinates, etc of drawing the bitmaps can be stuck here.
class ImageEditRenderer
{
	// these can construct an entire view
	struct ViewParams
	{
		ViewParams() :
			clientWidth(0),
			clientHeight(0),
			hasSelection(false)
		{
		}
		Viewport view;// panning & zoom.

		int clientWidth;
		int clientHeight;

		bool hasSelection;
		CRect selectionRect;
	};
public:
	ImageEditRenderer() :
		m_original(0),
		hwndSlave(0),
		hwnd(0)
	{
		SetDirty();
	}
	void SetHWND(HWND h)
	{
		hwnd = h;
		RECT rc;
		GetClientRect(hwnd, &rc);
		m_queued.clientWidth = rc.right;
		m_queued.clientHeight = rc.bottom;
		SetDirty();
	}
	void SetSlaveHWND(HWND h)
	{
		hwndSlave = h;
		SetDirty();
	}
	void SetOriginalImage(AnimBitmap<32>& o)
	{
		m_original = &o;
		SetDirty();
	}

	// ideally this should be done automatically somehow, or at least i should accept a rect.
	void Invalidate()
	{
		SetDirty();
		InvalidateAll();
	}

	const Viewport& GetViewport() const { return m_queued.view; }
	bool HasSelection() const { return m_queued.hasSelection; }
	CRect GetSelection() const { return m_queued.selectionRect; }

	// these are all functions which can affect the cached bitmaps. the trick will be to reduce the amount of image that's generated, and reduce
	// the times a re-cache needs to be done at all.
	void ClearSelection()
	{
		if(m_queued.hasSelection != false)
		{
			SelectionDirty = true;
			m_queued.hasSelection = false;

			InvalidateAll();
		}
	}
	void SetSelectionRect(const CRect& imgCoords)
	{
		//OutputDebugString(LibCC::Format("SetSelectionRect(%)|")(RectToString(imgCoords)).CStr());
		if((imgCoords.Width() < 1) || (imgCoords.Height() < 1))
		{
			ClearSelection();
			return;
		}

		if(!m_queued.hasSelection || m_queued.selectionRect != imgCoords)
		{
			SelectionDirty = true;
			m_queued.hasSelection = true;
			m_queued.selectionRect = imgCoords;

			InvalidateAll();
		}
	}
	void SetZoomFactor(ViewPortSubPixel z)
	{
		if(abs(m_queued.view.GetZoomFactor() - z) > 0.001)
		{
			ZoomDirty = true;
			m_queued.view.SetZoomFactor(z);

			InvalidateAll();
		}
	}
	void SetViewOrigin(const PointF& o)
	{
		if(!o.IsEqual(m_queued.view.GetViewOrigin(), 0.001))
		{
			PanningDirty = true;
			m_queued.view.SetViewOrigin(o);
			
			InvalidateAll();
		}
	}
	void SetImageOrigin(const PointF& o, const char* reason)
	{
		//LibCC::g_pLog->Message(LibCC::Format("SetImageOrigin(%,%) : %|")(o.x)(o.y)(reason).CStr());

		if(!o.IsEqual(m_queued.view.GetImageOrigin(), 0.001))
		{
			PanningDirty = true;
			m_queued.view.SetImageOrigin(o);

			InvalidateAll();
		}
	}
	void SetClientSize(int width, int height)
	{
		if(m_queued.clientHeight != height || m_queued.clientWidth != width)
		{
			ClientSizeDirty = true;
			m_queued.clientWidth = width;
			m_queued.clientHeight = height;
		}
	}

	/////////////////////////////////////////////////
	void Render(AnimBitmap<32>& dest, const CRect& rcArea)
	{
		CalculateRenderingValues();
		//OutputDebugString(LibCC::Format("Render|").CStr());

		/*
			cases:
			#1- if nothing changed, nothing!
			#2- if zoom changed, everything gets redrawn.
			#3- if panning changed or client size changed (and branch based on selection changed)
			#4- if panning and client size did NOT change, but selection changed
		*/

		if(ZoomDirty || PanningDirty || ClientSizeDirty)
		{
			RenderZoomed(rcArea);
			RenderGrayed(rcArea);
			RenderOffscreen(rcArea);
		}
		else if(SelectionDirty)
		{
			RenderGrayed(rcArea);
			RenderOffscreen(rcArea);
		}

		//OutputDebugString(LibCC::Format(
		//	"AfterRender.|"
		//	"  Client screen coords:                  (%,%)|"
		//	"  Original image coords:                 (%,%)|"
		//	"  Zoom level:                            %|"
		//	"  Visible image origin:                  %|"
		//	"  CalculateZoomedBufferImageCoords       %|"
		//	"  CalculateZoomedBufferScreenCoords      %|"
		//	"  CalculateZoomedBufferSize              %||"
		//	"  CalculateZoomedVisibleImageCoords      %|"
		//	"  CalculateZoomedVisibleScreenCoords     %|"
		//	"  CalculateZoomedVisibleBufferCoords     %|"
		//	"  CalculateZoomedVisibleSize             %||"
		//	"  CalculateSelectionCoordsOfZoomedBuffer %|"
		//	"  CalculateSelectionCoordsOfScreen       %|"
		//	)
		//	(m_queued.clientWidth)
		//	(m_queued.clientHeight)
		//	(m_original->GetWidth())
		//	(m_original->GetHeight())

		//	(m_queued.view.GetZoomFactor())
		//	(PointToString(m_queued.view.ViewToImage(PointF(0, 0))))
		//	(RectToString(zoomedBufferImageCoords))
		//	(RectToString(zoomedBufferScreenCoords))
		//	(SizeToString(zoomedBufferScreenCoords.Size()))
		//	(RectToString(zoomedVisibleImageCoords))
		//	(RectToString(zoomedVisibleScreenCoords))
		//	(RectToString(zoomedVisibleBufferCoords))
		//	(SizeToString(zoomedVisibleScreenCoords.Size()))
		//	(RectToString(selectionZoomedCoords))
		//	(RectToString(selectionScreenCoords))
		//	.CStr()
		//	);

		m_params = m_queued;
		SetDirty(false);
		m_offscreen.Blit(dest, rcArea.left, rcArea.top, rcArea.Width(), rcArea.Height(), rcArea.left, rcArea.top);
	}

private:
	void InvalidateAll()
	{
		if(!hwnd)
			return;

		RECT rc;
		GetClientRect(hwnd, &rc);
		InvalidateRect(hwnd, &rc, FALSE);

		if(!hwndSlave)
			return;
		GetClientRect(hwndSlave, &rc);
		InvalidateRect(hwndSlave, &rc, FALSE);
	}

	inline static void ClampRect(CRect& rc, const CRect& constraint)
	{
		if(rc.left < constraint.left) rc.left = constraint.left;
		if(rc.left > constraint.right) rc.left = constraint.right;
		if(rc.right < constraint.left) rc.right = constraint.left;
		if(rc.right > constraint.right) rc.right = constraint.right;
		if(rc.top < constraint.top) rc.top = constraint.top;
		if(rc.top > constraint.bottom) rc.top = constraint.bottom;
		if(rc.bottom < constraint.top) rc.bottom = constraint.top;
		if(rc.bottom > constraint.bottom) rc.bottom = constraint.bottom;
	}
	static inline bool IsBigEnough(const AnimBitmap<32>& b, int x, int y)
	{
		if(b.GetWidth() < x) return false;
		if(b.GetHeight() < y) return false;
		return true;
	}
	static inline void MakeBigEnough(AnimBitmap<32>& b, int x, int y)
	{
		if(!IsBigEnough(b, x, y))
		{
			b.SetSize(x, y);
		}
	}
	inline void ClampToImage(PointF& p) const
	{
		if(p.x < 0) p.x = 0;
		if(p.y < 0) p.y = 0;
		if(p.x > m_original->GetWidth()) p.x = m_original->GetWidth();
		if(p.y > m_original->GetHeight()) p.y = m_original->GetHeight();
	}
	inline void ClampToImage(RectF& rc) const
	{
		ClampToImage(rc.ul);
		ClampToImage(rc.br);
	}

	// a bunch of rendering values.
	struct RenderingValues
	{
		CRect zoomedBufferImageCoords;
		CRect zoomedBufferScreenCoords;
		CRect zoomedVisibleImageCoords;
		CRect zoomedVisibleScreenCoords;
		CRect zoomedVisibleBufferCoords;
		CRect selectionZoomedCoords;
		CRect selectionScreenCoords;
	};
	RenderingValues oldRP;
	RenderingValues newRP;

	void CalculateRenderingValues()
	{
		static const int zoomedBuffer = 25;// in screen pixels
		RectF ret;

		// calculate zoomedBufferImageCoords
		// zoomedBufferScreenCoords
		ret.Assign(-zoomedBuffer, -zoomedBuffer, m_queued.clientWidth + zoomedBuffer, m_queued.clientHeight + zoomedBuffer);
		ret = m_queued.view.ViewToImage(ret);
		ClampToImage(ret);
		// now we have image coords. StretchBlt cannot blit from sub-pixels, so...
		newRP.zoomedBufferImageCoords = ret.QuantizeInflate();
		newRP.zoomedBufferScreenCoords = m_queued.view.ImageToView(RectF(newRP.zoomedBufferImageCoords)).Round();

		// calculate zoomedVisibleImageCoords
		// zoomedVisibleScreenCoords
		ret.Assign(0, 0, m_queued.clientWidth, m_queued.clientHeight);
		ret = m_queued.view.ViewToImage(ret);
		ClampToImage(ret);
		newRP.zoomedVisibleImageCoords = ret.QuantizeInflate();
		newRP.zoomedVisibleScreenCoords = m_queued.view.ImageToView(RectF(newRP.zoomedVisibleImageCoords)).Round();

		// set up a method of converting screen coords to coords relative to the zoomed buffer.
		Viewport v;// "image" = buffer. "view" = screen.
		v.SetZoomFactor(1.0);
		v.SetImageOrigin(PointF(0, 0));
		v.SetViewOrigin(PointF(newRP.zoomedBufferScreenCoords.TopLeft()));

			// calculate zoomedVisibleBufferCoords
		newRP.zoomedVisibleBufferCoords = v.ViewToImage(RectF(newRP.zoomedVisibleScreenCoords)).Round();

		// calculate selectionZoomedCoords and selectionScreenCoords
		newRP.selectionScreenCoords = m_queued.view.ImageToView(RectF(m_queued.selectionRect)).Round();
		newRP.selectionZoomedCoords = v.ViewToImage(RectF(newRP.selectionScreenCoords)).Round();
	}

	inline bool PointIsBetween(int pt, int bound1, int bound2)
	{
		return (pt > min(bound1, bound2)) && (pt < max(bound1, bound2));
	}

	// returns true if any of source is within the display.
	inline bool RectIsRelevant(const CRect& rcDisplay, const CRect& source)
	{
		// for source to be visible, EITHER x coordinate must be within the display, AND EITHER y coordinate must be within the display.
		bool anXcoordIsWithin = PointIsBetween(source.left, rcDisplay.left, rcDisplay.right) || PointIsBetween(source.right, rcDisplay.left, rcDisplay.right);
		bool aYcoordIsWithin = PointIsBetween(source.top, rcDisplay.top, rcDisplay.bottom) || PointIsBetween(source.bottom, rcDisplay.top, rcDisplay.bottom);
		return anXcoordIsWithin && aYcoordIsWithin;
	}

	// TODO: only draw what we need to (using rcArea)
	void RenderZoomed(const CRect& rcArea)
	{
		MakeBigEnough(m_zoomed, newRP.zoomedBufferScreenCoords.Width(), newRP.zoomedBufferScreenCoords.Height());
		//m_zoomed.Fill(MakeRgbPixel32(255, 0, 0));

		int mode = m_queued.view.GetZoomFactor() >= 1.0 ? COLORONCOLOR : HALFTONE;
		m_original->StretchBlit(m_zoomed,
			0,
			0,
			newRP.zoomedBufferScreenCoords.Width(),
			newRP.zoomedBufferScreenCoords.Height(),
			newRP.zoomedBufferImageCoords.left,
			newRP.zoomedBufferImageCoords.top,
			newRP.zoomedBufferImageCoords.Width(),
			newRP.zoomedBufferImageCoords.Height(),
			mode);
	}

	// TODO: only draw what we need to (using rcArea)
	void RenderGrayed(const CRect& rcArea)
	{
		//// simple rendering code for debugging purposes.
		//MakeBigEnough(m_zoomedGrayed, zoomedBufferScreenCoords.Width(), zoomedBufferScreenCoords.Width());
		//m_zoomed.Blit(m_zoomedGrayed, 0, 0, zoomedBufferScreenCoords.Width(), zoomedBufferScreenCoords.Width());
		//m_zoomedGrayed.GrayOut();
		//return;

		if(!m_queued.hasSelection)
		{
			m_cachedSelectionArea.SetRectEmpty();
		}
		else
		{
			// cache the area that's grayed AND that isn't already cached.
			MakeBigEnough(m_zoomedGrayed, newRP.zoomedBufferScreenCoords.Width(), newRP.zoomedBufferScreenCoords.Height());
			//m_zoomedGrayed.Fill(MakeRgbPixel32(0,255,0));

			m_cachedSelectionArea = m_queued.selectionRect;

			SubtractRectHelper s(CRect(0, 0, newRP.zoomedBufferScreenCoords.Width(), newRP.zoomedBufferScreenCoords.Height()), newRP.selectionZoomedCoords);
			//if(RectIsRelevant(rcArea, s.top))
				m_zoomed.Blit(m_zoomedGrayed, s.top);
			//if(RectIsRelevant(rcArea, s.left))
				m_zoomed.Blit(m_zoomedGrayed, s.left);
			//if(RectIsRelevant(rcArea, s.right))
				m_zoomed.Blit(m_zoomedGrayed, s.right);
			//if(RectIsRelevant(rcArea, s.bottom))
				m_zoomed.Blit(m_zoomedGrayed, s.bottom);

			//if(RectIsRelevant(rcArea, s.top))
				m_zoomedGrayed.GrayRect(s.top);
			//if(RectIsRelevant(rcArea, s.left))
				m_zoomedGrayed.GrayRect(s.left);
			//if(RectIsRelevant(rcArea, s.right))
				m_zoomedGrayed.GrayRect(s.right);
			//if(RectIsRelevant(rcArea, s.bottom))
				m_zoomedGrayed.GrayRect(s.bottom);
		}
	}

	// TODO: only draw what we need to (using rcArea)
	void RenderOffscreen(const CRect& rcArea)
	{
		MakeBigEnough(m_offscreen, m_queued.clientWidth, m_queued.clientHeight);
		//m_offscreen.Fill(MakeRgbPixel32(0,0,255));

		// blit.
		if(!m_queued.hasSelection)
		{
			m_zoomed.Blit(
				m_offscreen,
				newRP.zoomedVisibleScreenCoords.left,
				newRP.zoomedVisibleScreenCoords.top,
				newRP.zoomedVisibleScreenCoords.Width(),
				newRP.zoomedVisibleScreenCoords.Height(),
				newRP.zoomedVisibleBufferCoords.left,
				newRP.zoomedVisibleBufferCoords.top);
		}
		else
		{
			m_zoomed.Blit(m_offscreen,
				newRP.selectionScreenCoords.left,
				newRP.selectionScreenCoords.top,
				newRP.selectionScreenCoords.Width(),
				newRP.selectionScreenCoords.Height(),
				newRP.selectionZoomedCoords.left,
				newRP.selectionZoomedCoords.top);

			SubtractRectHelper sZoomed(newRP.zoomedVisibleBufferCoords, newRP.selectionZoomedCoords);
			SubtractRectHelper sScreen(newRP.zoomedVisibleScreenCoords, newRP.selectionScreenCoords);

			m_zoomedGrayed.Blit(m_offscreen, sScreen.top.left, sScreen.top.top, sScreen.top.Width(), sScreen.top.Height(), sZoomed.top.left, sZoomed.top.top);
			m_zoomedGrayed.Blit(m_offscreen, sScreen.left.left, sScreen.left.top, sScreen.left.Width(), sScreen.left.Height(), sZoomed.left.left, sZoomed.left.top);
			m_zoomedGrayed.Blit(m_offscreen, sScreen.right.left, sScreen.right.top, sScreen.right.Width(), sScreen.right.Height(), sZoomed.right.left, sZoomed.right.top);
			m_zoomedGrayed.Blit(m_offscreen, sScreen.bottom.left, sScreen.bottom.top, sScreen.bottom.Width(), sScreen.bottom.Height(), sZoomed.bottom.left, sZoomed.bottom.top);
		}

		// draw checkers
		m_offscreen.FillCheckerPatternExclusion(newRP.zoomedVisibleScreenCoords, m_queued.hasSelection);

		// draw selection handles
		if(m_queued.hasSelection)
		{
			//AnimBitmap<32>::RectInverseSafeParams params;
			//params.src = &m_zoomed;
			//params.srcOffsetX = -newRP.zoomedVisibleScreenCoords.left;
			//params.srcOffsetY = -newRP.zoomedVisibleScreenCoords.top;

			int SelectionHandleWidth = 3;// only for the corners
			//int SelectionHandleWidthBorder = 6;// only for the border handles (left/top/right/bottom)
			int SelectionHandleLegX = min(9, newRP.selectionScreenCoords.Width());
			int SelectionHandleLegY = min(9, newRP.selectionScreenCoords.Height());

			// Upper-left
			m_offscreen.RectSmallCheckerSafe(
				newRP.selectionScreenCoords.left - SelectionHandleWidth,
				newRP.selectionScreenCoords.top - SelectionHandleWidth,
				SelectionHandleLegX + SelectionHandleWidth,
				SelectionHandleWidth);// top leg

			m_offscreen.RectSmallCheckerSafe(
				newRP.selectionScreenCoords.left - SelectionHandleWidth,
				newRP.selectionScreenCoords.top,
				SelectionHandleWidth,
				SelectionHandleLegY);// left leg

			// Lower-left
			m_offscreen.RectSmallCheckerSafe(
				newRP.selectionScreenCoords.left - SelectionHandleWidth,
				newRP.selectionScreenCoords.bottom,
				SelectionHandleLegX + SelectionHandleWidth,
				SelectionHandleWidth);// bottom leg

			m_offscreen.RectSmallCheckerSafe(
				newRP.selectionScreenCoords.left - SelectionHandleWidth,
				newRP.selectionScreenCoords.bottom - SelectionHandleLegY,
				SelectionHandleWidth,
				SelectionHandleLegY);// left leg

			// Lower-right
			m_offscreen.RectSmallCheckerSafe(
				newRP.selectionScreenCoords.right - SelectionHandleLegX,
				newRP.selectionScreenCoords.bottom,
				SelectionHandleLegX + SelectionHandleWidth,
				SelectionHandleWidth);// bottom leg

			m_offscreen.RectSmallCheckerSafe(
				newRP.selectionScreenCoords.right,
				newRP.selectionScreenCoords.bottom - SelectionHandleLegY,
				SelectionHandleWidth,
				SelectionHandleLegY);// right leg

			// Upper-right
			m_offscreen.RectSmallCheckerSafe(
				newRP.selectionScreenCoords.right - SelectionHandleLegX,
				newRP.selectionScreenCoords.top - SelectionHandleWidth,
				SelectionHandleLegX + SelectionHandleWidth,
				SelectionHandleWidth);// top leg

			m_offscreen.RectSmallCheckerSafe(
				newRP.selectionScreenCoords.right,
				newRP.selectionScreenCoords.top,
				SelectionHandleWidth,
				SelectionHandleLegY);// right leg

			// top
			//CPoint middles = newRP.selectionScreenCoords.CenterPoint();
			//middles.x -= SelectionHandleLegX / 2;
			//middles.y -= SelectionHandleLegY / 2;

			//m_offscreen.RectSmallCheckerSafe(
			//	middles.x,
			//	newRP.selectionScreenCoords.top - SelectionHandleWidthBorder,
			//	SelectionHandleLegX,
			//	SelectionHandleWidthBorder);

			//// right
			//m_offscreen.RectSmallCheckerSafe(
			//	newRP.selectionScreenCoords.right,
			//	middles.y,
			//	SelectionHandleWidthBorder,
			//	SelectionHandleLegY);

			//// bottom
			//m_offscreen.RectSmallCheckerSafe(
			//	middles.x,
			//	newRP.selectionScreenCoords.bottom,
			//	SelectionHandleLegX,
			//	SelectionHandleWidthBorder);

			//// left
			//m_offscreen.RectSmallCheckerSafe(
			//	newRP.selectionScreenCoords.left - SelectionHandleWidthBorder,
			//	middles.y,
			//	SelectionHandleWidthBorder,
			//	SelectionHandleLegY);
		}

		oldRP = newRP;
	}

	AnimBitmap<32>* m_original;

	AnimBitmap<32> m_zoomed;

	AnimBitmap<32> m_zoomedGrayed;
	CRect m_cachedSelectionArea;// describes the area which is SUROUNDED by grayed area in the cached bitmap. if this is the entire rect of the zoomedGrayed bitmap, then no graying has been done. This is valid even if m_hasSelection is false!!

	AnimBitmap<32> m_offscreen;// copy of what should be blitted to the destination... should always be "fresh" ready to be painted to the screen.

	HWND hwnd;
	HWND hwndSlave;
	ViewParams m_params;// the currently-cached parameters
	ViewParams m_queued;// these are worked on until rendering is requested. then caches are updated if necessary, and values are copied to m_params.
	bool ZoomDirty;
	bool PanningDirty;
	bool ClientSizeDirty;
	bool SelectionDirty;

	void SetDirty(bool b = true)
	{
		ZoomDirty = b;
		PanningDirty = b;
		ClientSizeDirty = b;
		SelectionDirty = b;
		if(b)
			InvalidateAll();
	}
};


