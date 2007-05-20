
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
	void SetOriginalImage(AnimBitmap<32>& o)
	{
		m_original = &o;
		SetDirty();
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
		//OutputDebugString(LibCC::Format("SetImageOrigin(%,%) : %|")(o.x)(o.y)(reason).CStr());

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
			RenderZoomed();
			RenderGrayed();
			RenderOffscreen();
		}
		else if(SelectionDirty)
		{
			RenderGrayed();
			RenderOffscreen();
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
		RECT rc;
		GetClientRect(hwnd, &rc);
		InvalidateRect(hwnd, &rc, FALSE);
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
	CRect zoomedBufferImageCoords;
	CRect zoomedBufferScreenCoords;
	CRect zoomedVisibleImageCoords;
	CRect zoomedVisibleScreenCoords;
	CRect zoomedVisibleBufferCoords;
	CRect selectionZoomedCoords;
	CRect selectionScreenCoords;

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
		zoomedBufferImageCoords = ret.QuantizeInflate();
		zoomedBufferScreenCoords = m_queued.view.ImageToView(RectF(zoomedBufferImageCoords)).Round();

		// calculate zoomedVisibleImageCoords
		// zoomedVisibleScreenCoords
		ret.Assign(0, 0, m_queued.clientWidth, m_queued.clientHeight);
		ret = m_queued.view.ViewToImage(ret);
		ClampToImage(ret);
		zoomedVisibleImageCoords = ret.QuantizeInflate();
		zoomedVisibleScreenCoords = m_queued.view.ImageToView(RectF(zoomedVisibleImageCoords)).Round();

		// set up a method of converting screen coords to coords relative to the zoomed buffer.
		Viewport v;// "image" = buffer. "view" = screen.
		v.SetZoomFactor(1.0);
		v.SetImageOrigin(PointF(0, 0));
		v.SetViewOrigin(PointF(zoomedBufferScreenCoords.TopLeft()));

			// calculate zoomedVisibleBufferCoords
		zoomedVisibleBufferCoords = v.ViewToImage(RectF(zoomedVisibleScreenCoords)).Round();

		// calculate selectionZoomedCoords and selectionScreenCoords
		selectionScreenCoords = m_queued.view.ImageToView(RectF(m_queued.selectionRect)).Round();
		selectionZoomedCoords = v.ViewToImage(RectF(selectionScreenCoords)).Round();
	}

	void RenderZoomed()
	{
		MakeBigEnough(m_zoomed, zoomedBufferScreenCoords.Width(), zoomedBufferScreenCoords.Height());
		m_zoomed.Fill(MakeRgbPixel32(255, 0, 0));

		int mode = m_queued.view.GetZoomFactor() >= 1.0 ? COLORONCOLOR : HALFTONE;
		m_original->StretchBlit(m_zoomed,
			0,
			0,
			zoomedBufferScreenCoords.Width(),
			zoomedBufferScreenCoords.Height(),
			zoomedBufferImageCoords.left,
			zoomedBufferImageCoords.top,
			zoomedBufferImageCoords.Width(),
			zoomedBufferImageCoords.Height(),
			mode);
	}

	void RenderGrayed()
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
			MakeBigEnough(m_zoomedGrayed, zoomedBufferScreenCoords.Width(), zoomedBufferScreenCoords.Height());
			m_zoomedGrayed.Fill(MakeRgbPixel32(0,255,0));

			m_cachedSelectionArea = m_queued.selectionRect;

			SubtractRectHelper s(CRect(0, 0, zoomedBufferScreenCoords.Width(), zoomedBufferScreenCoords.Height()), selectionZoomedCoords);
			m_zoomed.Blit(m_zoomedGrayed, s.top);
			m_zoomed.Blit(m_zoomedGrayed, s.left);
			m_zoomed.Blit(m_zoomedGrayed, s.right);
			m_zoomed.Blit(m_zoomedGrayed, s.bottom);

			m_zoomedGrayed.GrayRect(s.top);
			m_zoomedGrayed.GrayRect(s.left);
			m_zoomedGrayed.GrayRect(s.right);
			m_zoomedGrayed.GrayRect(s.bottom);
		}
	}

	void RenderOffscreen()
	{
		MakeBigEnough(m_offscreen, m_queued.clientWidth, m_queued.clientHeight);
		m_offscreen.Fill(MakeRgbPixel32(0,0,255));

		// blit.
		if(!m_queued.hasSelection)
		{
			m_zoomed.Blit(
				m_offscreen,
				zoomedVisibleScreenCoords.left,
				zoomedVisibleScreenCoords.top,
				zoomedVisibleScreenCoords.Width(),
				zoomedVisibleScreenCoords.Height(),
				zoomedVisibleBufferCoords.left,
				zoomedVisibleBufferCoords.top);
		}
		else
		{
			m_zoomed.Blit(m_offscreen,
				selectionScreenCoords.left,
				selectionScreenCoords.top,
				selectionScreenCoords.Width(),
				selectionScreenCoords.Height(),
				selectionZoomedCoords.left,
				selectionZoomedCoords.top);

			SubtractRectHelper sZoomed(zoomedVisibleBufferCoords, selectionZoomedCoords);
			SubtractRectHelper sScreen(zoomedVisibleScreenCoords, selectionScreenCoords);

			m_zoomedGrayed.Blit(m_offscreen, sScreen.top.left, sScreen.top.top, sScreen.top.Width(), sScreen.top.Height(), sZoomed.top.left, sZoomed.top.top);
			m_zoomedGrayed.Blit(m_offscreen, sScreen.left.left, sScreen.left.top, sScreen.left.Width(), sScreen.left.Height(), sZoomed.left.left, sZoomed.left.top);
			m_zoomedGrayed.Blit(m_offscreen, sScreen.right.left, sScreen.right.top, sScreen.right.Width(), sScreen.right.Height(), sZoomed.right.left, sZoomed.right.top);
			m_zoomedGrayed.Blit(m_offscreen, sScreen.bottom.left, sScreen.bottom.top, sScreen.bottom.Width(), sScreen.bottom.Height(), sZoomed.bottom.left, sZoomed.bottom.top);
		}

		// draw checkers
		m_offscreen.FillCheckerPatternExclusion(zoomedVisibleScreenCoords, m_queued.hasSelection);
	}

	AnimBitmap<32>* m_original;

	AnimBitmap<32> m_zoomed;

	AnimBitmap<32> m_zoomedGrayed;
	CRect m_cachedSelectionArea;// describes the area which is SUROUNDED by grayed area in the cached bitmap. if this is the entire rect of the zoomedGrayed bitmap, then no graying has been done. This is valid even if m_hasSelection is false!!

	AnimBitmap<32> m_offscreen;// copy of what should be blitted to the destination... should always be "fresh" ready to be painted to the screen.

	HWND hwnd;
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
	}
};


