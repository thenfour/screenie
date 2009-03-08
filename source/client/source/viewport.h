// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark


#ifndef VIEWPORT_INCLUDED
#define VIEWPORT_INCLUDED

#include "fundamental.h"


/*
  Class to encapsulate translating between a image and a view into it (e.g. a huge image & the window that displays part of it).
  items this takes into consideration:
  1) view origin (where in the viewport should the image's origin be centered on?)
  2) image origin (where in the image is the viewport's origin centered on?)
  3) view rect
  4) view zoom

		the origins are used to line the two "canvases" on top of eachother.
		Let's break this down into a single dimension instead of 2:
		|------------------|  This is the width of the window.
		|-------------------------------| This is the width of the image.
		Because the image is not always butted exactly up to the window's 0 coordinate, we need a way to calibrate them.
		One way was to have a single "offset".
		          |------------------|  This is the width of the window.
		|-------------------------------| This is the width of the image.
		^---------^ This is the offset.

		This is fine, and callers must pass in the offset.
		So, considering that there's a zoom factor involved, is "offset" image coordinates or view coordinates?
		It's easier for callers to simply pass in the two numbers which are
		used to derive the origin: view origin & image origin.
		The SPECIFIC scenario where this is really necessary is the combination of zooming & panning.
		If I zoom into some area in the left corner, applications can simply say "line up these suckers at the point where the cursor is.

		          |------*-----------|  This is the width of the window.
							^------^ This is the view origin. it's in view coordinates.
		|----------------*--------------| This is the width of the image.
		^----------------^ This is the image origin.

		You see that you could easily have the same visual with
		different origins. This example would look the same as above.

		          |-------------*----|  This is the width of the window.
							^-------------^ This is the view origin. it's in view coordinates.
		|-----------------------*-------| This is the width of the image.
		^-----------------------^ This is the image origin.

*/
class Viewport
{
public:

  Viewport() :
    zoomFactor(1.0f)
  {
  }

  Viewport(const Viewport& rhs) :
    imageOrigin(rhs.imageOrigin),
    viewOrigin(rhs.viewOrigin),
    zoomFactor(rhs.zoomFactor)
  {
  }

  Viewport& operator =(const Viewport& rhs)
  {
    imageOrigin = rhs.imageOrigin;
    viewOrigin = rhs.viewOrigin;
    zoomFactor = rhs.zoomFactor;
		return *this;
  }

  void SetZoomFactor(ViewPortSubPixel z)
  {
    zoomFactor = z;
  }
  ViewPortSubPixel GetZoomFactor() const
  {
    return zoomFactor;
  }
  void SetViewOrigin(PointF o)
  {
    viewOrigin = o;
  }
  const PointF& GetViewOrigin() const
  {
    return viewOrigin;
  }
  void SetImageOrigin(PointF o)
  {
    imageOrigin = o;
  }
  const PointF& GetImageOrigin() const
  {
    return imageOrigin;
  }

	// ********************************************** CONVERSIONS (the reason this class exists)
  PointF ViewToImage(const PointF& p) const
  {
		/*
		|VIEW:                         |-----*----------.----|      <-- definition of the view origin
		|SCALED IMAGE (2x):  |---------------*----------.-------------------------|
		|IMAGE:                      |-------*-----.-----------|    <-- definition of the image origin
		|
		|input:                        <---------------->
		|The calculation is:
		|                                    <----------> input minus view origin
		|                                    <-----> divide by zoom
		|                            <-------> image origin
		|output:                     <-------------> (add them together)
		|
			textual description of this process:
			1) find the distance from the input to the view origin
			2) scale it down. now it's the distance to the image origin
			3) offset that by the image origin.
		*/
		PointF ret;
		ret.x = p.x - viewOrigin.x;
		ret.x /= zoomFactor;
		ret.x += imageOrigin.x;

		ret.y = p.y - viewOrigin.y;
		ret.y /= zoomFactor;
		ret.y += imageOrigin.y;

		return ret;
  }
  PointF ImageToView(const PointF& p) const
  {
		// this is quite literally the inverse of the above.
		// 1) make it relative to the origin
		// 2) scale it to the new coords
		// 3) offset it
		PointF ret;
		ret.x = p.x - imageOrigin.x;
		ret.x *= zoomFactor;
		ret.x += viewOrigin.x;

		ret.y = p.y - imageOrigin.y;
		ret.y *= zoomFactor;
		ret.y += viewOrigin.y;
		return ret;
  }
	// these do not take into consider the origins. they just consider zoom.
  PointF ViewToImageSize(PointF p) const
  {
    return PointF(p.x / zoomFactor, p.y / zoomFactor);
  }
  PointF ImageToViewSize(PointF p) const
  {
    return PointF(zoomFactor * p.x, zoomFactor * p.y);
  }

	RectF ViewToImage(const RectF& rc) const
	{
		RectF ret;
		ret.ul = ViewToImage(rc.ul);
		ret.br = ViewToImage(rc.br);
		return ret;
	}

	RectF ImageToView(const RectF& rc) const
	{
		RectF ret;
		ret.ul = ImageToView(rc.ul);
		ret.br = ImageToView(rc.br);
		return ret;
	}

protected:
  // basic members
  PointF viewOrigin;
  PointF imageOrigin;
  ViewPortSubPixel zoomFactor;
};

#endif

