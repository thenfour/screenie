/*
	let's try caching bitmaps at almost every stage of render...
	here are the stages of rendering:
	1) original image
	   this is irelevant to caching; it never changes.
	2) zoomed original image
	   this is a huge candidate for caching because it doesn't change very much, and when it does,
	   users won't be pissed it if shows a performance diference. HOWEVER, the problem is that i can
	   only cache a window of the image, because at high zoom levels it would mean very very large cached
	   images. So the best way to cache this is going to be to only cache a "window" of the original image.
	   Now there will be non-constant performance while panning... so we will need to cache a window that's bigger
	   in general than the visible area. that way while you are panning, there will be a buffer which will
	   reduce the number of refreshes. this logic will be in a separate class so i don't get confused =)
	3) selection overlay (grayed out)
   definitely should be cached. this will be simply a copy of the zoomed original image, with a gray overlay.
	   it might need to be lazy-cached or else it would result in #2 being well over 2x as much processing. also,
	   if you think about it, when there is grayed out area, it is rarely the entire image. so it might make sense
	   to also lazy-cache AREAS of the grayed area. If that's necessary i'd do it later.
	4) ants marching.
	   this could be quite difficult to cache because it's an animation. there are only 2 states of each pixel in
	   the animation, but unfortunately it would need to be determined if a ton of BitBlt's is going to be faster
	   than just dealing with pixel-by-pixel.
	5) other tools... to be written of course. i'm assuming that there would be 2 or more different stages of
	   rendering though for tools. stuff could be rendered as part of the original image, or as part of the tool
	   layer, like selection.

	   for changes to the original image, obviously that would mean re-caching all these images for each change...
	   for example, if you have a selection (so, a grayed area), and you have some pencil tool. If you are drawing
	   around with it, the pencil tool would need to draw to all the cached bitmaps as well as the original.

	   We will address that when it comes up.
*/

#pragma once

#include "image.hpp"

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
	void SetImageOrigin(const PointF& o)
	{
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
		/*
			cases:
			#1- if nothing changed, nothing!
			#2- if zoom changed, everything gets redrawn.
			#3- if panning changed or client size changed (and branch based on selection changed)
			#4- if panning and client size did NOT change, but selection changed
		*/

		// NOTE: the checkered background will be applied to the offscreen bitmap directly, after other stuff has been drawn.

		const int zoomedBuffer = 25;// in client pixels

		if(true || ZoomDirty)
		{
			//OutputDebugString(LibCC::Format("Img(%,%) / View(%,%)|")
			//	(m_queued.view.GetImageOrigin().x)
			//	(m_queued.view.GetImageOrigin().y)
			//	(m_queued.view.GetViewOrigin().x)
			//	(m_queued.view.GetViewOrigin().y)
			//	.CStr());

			{// generate the zoomed bitmap.
				// get the image coords of the area we should cache.
				m_zoomedUL = m_queued.view.ViewToImage(PointF(-zoomedBuffer, -zoomedBuffer));
				m_zoomedBR = m_queued.view.ViewToImage(PointF(m_queued.clientWidth + zoomedBuffer, m_queued.clientHeight + zoomedBuffer));
				// don't cache stuff outside of the image.
				ClampToImage(m_zoomedUL);
				ClampToImage(m_zoomedBR);
				// since stretchblt only works on whole pixels, make all of them integral, erring on the side of being bigger.
				CPoint imgul = m_zoomedUL.Floor();
				CPoint imgbr = m_zoomedBR.Ceil();
				m_zoomedUL.SelfFloor();
				m_zoomedBR.SelfCeil();
				// couple more calculations and we're almost done
				int imgwidth = imgbr.x - imgul.x;
				int imgheight = imgbr.y - imgul.y;
				m_zoomedSize = m_queued.view.ImageToViewSize(PointF(imgwidth, imgheight));// store the non-integral size, for accuracy reasons.
				CPoint intDestSize = m_zoomedSize.Round();

				if((m_zoomed.GetWidth() < intDestSize.x) || (m_zoomed.GetHeight() < intDestSize.y))
				{
					m_zoomed.SetSize(intDestSize.x, intDestSize.y);
				}

				// blit!
				int mode = m_queued.view.GetZoomFactor() >= 1.0 ? COLORONCOLOR : HALFTONE;
				m_original->StretchBlit(m_zoomed,
					0,
					0,
					intDestSize.x,
					intDestSize.y,
					imgul.x,
					imgul.y,
					imgwidth,
					imgheight,
					mode);
			}

			{// now generate the zoomedgrayed

				//CPoint zoomedSize = m_zoomedSize.Round();
				//if((m_zoomedGrayed.GetWidth() < zoomedSize.x) || (m_zoomedGrayed.GetHeight() < zoomedSize.y))
				//{
				//	m_zoomedGrayed.SetSize(zoomedSize.x, zoomedSize.y);
				//}
				//m_zoomed.Blit(m_zoomedGrayed, 0, 0, zoomedSize.x, zoomedSize.y);

				if(!m_queued.hasSelection)
				{
					m_cachedSelectionArea.SetRectEmpty();
				}
				else
				{
					// cache the area that's grayed.
					CPoint zoomedSize = m_zoomedSize.Round();
					if((m_zoomedGrayed.GetWidth() < zoomedSize.x) || (m_zoomedGrayed.GetHeight() < zoomedSize.y))
					{
						m_zoomedGrayed.SetSize(zoomedSize.x, zoomedSize.y);
					}
					m_cachedSelectionArea = m_queued.selectionRect;

					// figure out where in m_zoomed the selection rect lies. easiest way to do this is use the Viewport to help
					// translate from image coords -> m_zoomedGrayed coords.
					CRect sel = GetSelectionCoordsOfZoomed();

					SubtractRectHelper s(CRect(0, 0, zoomedSize.x, zoomedSize.y), sel);
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

			{// blit from zoomed to offscreen
				// strategy here is 1) figure out what stuff we need. 2) find the respective coords in the zoomed cache, 3) blit.
				// so... here i find the image coords that bound the visible area.
				PointF imgul = m_queued.view.ViewToImage(PointF(0, 0));
				PointF imgbr = m_queued.view.ViewToImage(PointF(m_queued.clientWidth, m_queued.clientHeight));
				// don't care about coords outside of image
				ClampToImage(imgul);
				ClampToImage(imgbr);
				// calculate the easy destination coords
				CPoint destul = m_queued.view.ImageToView(imgul).Round();
				CPoint destbr = m_queued.view.ImageToView(imgbr).Round();
				CRect dest(destul, destbr);
				// figure out the starting point of the zoomed cache.
				PointF srculf(imgul.x - m_zoomedUL.x, imgul.y - m_zoomedUL.y);// calculate the area in the zoomed cache that we don't care about- right now in img coords
				CPoint srcul = m_queued.view.ImageToViewSize(srculf).Round();// now UL is in usable screen coords
				CRect src(srcul.x, srcul.y, srcul.x + dest.Width(), srcul.y + dest.Height());

				if((m_offscreen.GetWidth() != m_queued.clientWidth) || (m_offscreen.GetHeight() != m_queued.clientHeight))
				{
					m_offscreen.SetSize(m_queued.clientWidth, m_queued.clientHeight);
				}

				// blit.
				if(!m_queued.hasSelection)
				{
					m_zoomed.Blit(m_offscreen, dest.left, dest.top, dest.Width(), dest.Height(), src.left, src.top);
				}
				else
				{
					// figure out where in zoomed the selection is, and do the above blit but in 5 parts.
					CRect selsrc = GetSelectionCoordsOfZoomed();
					// get those same coords for the display.
					PointF seldestUL = m_queued.view.ImageToView(PointF(m_queued.selectionRect.TopLeft()));
					PointF seldestBR = m_queued.view.ImageToView(PointF(m_queued.selectionRect.BottomRight()));
					CRect seldest(seldestUL.Round(), seldestBR.Round());

					SubtractRectHelper subsrc(src, selsrc);
					SubtractRectHelper subdest(dest, seldest);

					m_zoomed.Blit(m_offscreen, seldest.left, seldest.top, seldest.Width(), seldest.Height(), selsrc.left, selsrc.top);
					m_zoomedGrayed.Blit(m_offscreen, subdest.top.left, subdest.top.top, subdest.top.Width(), subdest.top.Height(), subsrc.top.left, subsrc.top.top);
					m_zoomedGrayed.Blit(m_offscreen, subdest.left.left, subdest.left.top, subdest.left.Width(), subdest.left.Height(), subsrc.left.left, subsrc.left.top);
					m_zoomedGrayed.Blit(m_offscreen, subdest.right.left, subdest.right.top, subdest.right.Width(), subdest.right.Height(), subsrc.right.left, subsrc.right.top);
					m_zoomedGrayed.Blit(m_offscreen, subdest.bottom.left, subdest.bottom.top, subdest.bottom.Width(), subdest.bottom.Height(), subsrc.bottom.left, subsrc.bottom.top);
				}

				// draw checkers
				CRect exclusion(destul, destbr);
				m_offscreen.FillCheckerPatternExclusion(exclusion, m_queued.hasSelection);
			}
		}

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

	// gets the coords in m_zoomed which correspond to the selection rect.
	CRect GetSelectionCoordsOfZoomed() const
	{
		Viewport v(m_queued.view);
		v.SetImageOrigin(m_zoomedUL);// top-left coord of m_zoomed
		v.SetViewOrigin(PointF(0, 0));// top-left coord of m_zoomed
		PointF selUL = v.ImageToView(PointF(m_queued.selectionRect.TopLeft()));
		PointF selBR = v.ImageToView(PointF(m_queued.selectionRect.BottomRight()));
		CRect ret(selUL.Round(), selBR.Round());
		CPoint zoomedSize = m_zoomedSize.Round();
		ClampRect(ret, CRect(0, 0, zoomedSize.x, zoomedSize.y));
		return ret;
	}

	static inline bool IsBigEnough(const AnimBitmap<32>& b, int x, int y)
	{
		if(b.GetWidth() < x) return false;
		if(b.GetHeight() < y) return false;
		return true;
	}

	inline void ClampToImage(PointF& p) const
	{
		if(p.x < 0) p.x = 0;
		if(p.y < 0) p.y = 0;
		if(p.x > m_original->GetWidth()) p.x = m_original->GetWidth();
		if(p.y > m_original->GetHeight()) p.y = m_original->GetHeight();
	}
	AnimBitmap<32>* m_original;

	AnimBitmap<32> m_zoomed;
	PointF m_zoomedUL;// upper-left coord of the IMAGE coordinate where m_zoomed starts
	PointF m_zoomedBR;// bottom-right of the same thing.
	PointF m_zoomedSize;// since m_zoomed might actually be bigger than necessary, this specifies the size that's relevant, in screen units

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


