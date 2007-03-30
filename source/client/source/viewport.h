

#ifndef VIEWPORT_INCLUDED
#define VIEWPORT_INCLUDED

typedef double ViewPortSubPixel;

inline int Round(ViewPortSubPixel f)
{
  return static_cast<int>(f + 0.5);
}


class PointI
{
public:
  typedef PointI This_T;

  // Constructors
  PointI() :
    x(0),
    y(0)
  {
  }
  PointI(int initX, int initY) :
    x(initX),
    y(initY)
  {
  }
  PointI(const This_T& rhs) :
    x(rhs.x),
    y(rhs.y)
  {
  }

  This_T& operator =(const This_T& rhs)
  {
    x = rhs.x;
    y = rhs.y;
    return *this;
  }

  // Operations
	void Offset(int xOffset, int yOffset)
  {
    x += xOffset;
    y += yOffset;
  }

	void Assign(int X, int Y)
  {
    x = X;
    y = Y;
  }

  int x;
  int y;
};

class PointF
{
public:
  typedef PointF This_T;

  // Constructors
  PointF() :
    x(0.0f),
    y(0.0f)
  {
  }
  PointF(ViewPortSubPixel initX, ViewPortSubPixel initY) :
    x(initX),
    y(initY)
  {
  }
  PointF(const This_T& rhs) :
    x(rhs.x),
    y(rhs.y)
  {
  }

  This_T& operator =(const This_T& rhs)
  {
    x = rhs.x;
    y = rhs.y;
    return *this;
  }

  // Operations
	void Offset(ViewPortSubPixel xOffset, ViewPortSubPixel yOffset)
  {
    x += xOffset;
    y += yOffset;
  }

	void Assign(ViewPortSubPixel X, ViewPortSubPixel Y)
  {
    x = X;
    y = Y;
  }

  ViewPortSubPixel x;
  ViewPortSubPixel y;
};

inline PointI PointFtoI(PointF x)
{
  return PointI(Round(x.x), Round(x.y));
}


/*
  Class to encapsulate translating between a "virtual canvas" and a view into it (e.g. a huge image & the window that displays part of it).
  items this takes into consideration:
  1) view origin (where in the viewport should the canvas origin be centered on?)
  2) virtual origin (where in the virtual canvas is the viewport centered on?)
  3) view rect
  4) view zoom

  "View" coords are relative to the view origin.  This will most practically be WINDOW coords.
  "Virtual" coords are relative to the origin of the virtual canvas. This will most practically be IMAGE coords.
  "Zoomed" coords are relative to the origin of the virtual canvas, but are zoomed.  If zoom is 2, then 1 "Zoomed" coord = 2 non-zoomed coords.  Zoomed coords i don't think would ever be used out of this class.

  The virtual origin is where alignment happens on the virtual canvas.  You would modify this
  when you are panning around the image.

  The view origin is where you align things to the viewport itself.  This probably wont get modified.

  The way this class translates coordinates is basically i set the viewport origin directly on top of the
  canvas origin.  that gives me the orientation, and anything can be calculated off of that.
*/
class Viewport
{
public:

  Viewport() :
    zoomFactor(1.0f)
  {
  }

  Viewport(const Viewport& rhs) :
    virtualOrigin(rhs.virtualOrigin),
    viewOrigin(rhs.viewOrigin),
    zoomedOrigin(rhs.zoomedOrigin),
    zoomFactor(rhs.zoomFactor),
    viewOffsetFromZoomed(rhs.viewOffsetFromZoomed)
  {
  }

  Viewport& operator =(const Viewport& rhs)
  {
    virtualOrigin = rhs.virtualOrigin;
    viewOrigin = rhs.viewOrigin;
    zoomedOrigin = rhs.zoomedOrigin;
    zoomFactor = rhs.zoomFactor;
    viewOffsetFromZoomed = rhs.viewOffsetFromZoomed;
  }

  void SetZoomFactor(ViewPortSubPixel z)
  {
    zoomFactor = z;
    CacheValues();
  }
  ViewPortSubPixel GetZoomFactor() const
  {
    return zoomFactor;
  }
  void SetViewOrigin(PointF o)
  {
    viewOrigin = o;
    CacheValues();
  }
  const PointF& GetViewOrigin() const
  {
    return viewOrigin;
  }
  void SetVirtualOrigin(PointF o)
  {
    virtualOrigin = o;
    CacheValues();
  }
  const PointF& GetVirtualOrigin() const
  {
    return virtualOrigin;
  }

  PointI ViewToVirtual(PointI x) const
  {
    return _ZoomedToVirtual(_ViewToZoomed(x));
  }

  PointF ViewToVirtual(PointF x) const
  {
    return _ZoomedToVirtual(_ViewToZoomed(x));
  }

  PointI VirtualToView(PointI x) const
  {
    return _ZoomedToView(_VirtualToZoomed(x));
  }

  PointF VirtualToView(PointF x) const
  {
    return _ZoomedToView(_VirtualToZoomed(x));
  }

  PointI ViewToVirtualSize(PointI x) const
  {
    return _ZoomedToVirtualSize(x);
  }

  PointF ViewToVirtualSize(PointF x) const
  {
    return _ZoomedToVirtualSize(x);
  }

  PointI VirtualToViewSize(PointI x) const
  {
    return _VirtualToZoomedSize(x);
  }

  PointF VirtualToViewSize(PointF x) const
  {
    return _VirtualToZoomedSize(x);
  }

protected:
  PointI _ZoomedToVirtualSize(PointI x) const
  {
    return PointI(
      Round((static_cast<ViewPortSubPixel>(x.x) / zoomFactor)),
      Round((static_cast<ViewPortSubPixel>(x.y) / zoomFactor))
      );
  }
  PointF _ZoomedToVirtualSize(PointF x) const
  {
    return PointF(
      (static_cast<ViewPortSubPixel>(x.x) / zoomFactor),
      (static_cast<ViewPortSubPixel>(x.y) / zoomFactor)
      );
  }

  PointI _VirtualToZoomedSize(PointI x) const
  {
    return PointI(
      Round(zoomFactor * x.x),
      Round(zoomFactor * x.y)
      );
  }

  PointF _VirtualToZoomedSize(PointF x) const
  {
    return PointF(
      zoomFactor * x.x,
      zoomFactor * x.y
      );
  }

  PointI _ViewToZoomed(PointI x) const
  {
    /*              origins
                      |
      |===========x===*============================| zoomed virtual canvas
            |=====x===*===============|  viewport
            <-----> input
      <-----------> what we are calculating.
      <----> viewOffsetFromZoomed cached value.
    */
    return PointI(
      x.x + (int)viewOffsetFromZoomed.x,
      x.y + (int)viewOffsetFromZoomed.y
      );
  }

  PointF _ViewToZoomed(PointF x) const
  {
    return PointF(
      viewOffsetFromZoomed.x + x.x,
      viewOffsetFromZoomed.y + x.y
      );
  }

  PointI _ZoomedToView(PointI x) const
  {
    /*              origins
                      |
      |===========x===*============================| zoomed virtual canvas
            |=====x===*===============|  viewport
      <-----------> input
            <-----> what we are calculating.
      <----> viewOffsetFromZoomed cached value.
    */
    return PointI(
      (int)(x.x - viewOffsetFromZoomed.x),
      (int)(x.y - viewOffsetFromZoomed.y)
      );
  }
  PointF _ZoomedToView(PointF x) const
  {
    return PointF(
      x.x - viewOffsetFromZoomed.x,
      x.y - viewOffsetFromZoomed.y
      );
  }

  PointI _ZoomedToVirtual(PointI x) const
  {
    /*
      |=============x===========================| zoomed virtual canvas
      |=====x==============| virtual canvas
      <-------------> input
      <-----> return
    */
    return PointI(
      Round((static_cast<ViewPortSubPixel>(x.x) / zoomFactor)),
      Round((static_cast<ViewPortSubPixel>(x.y) / zoomFactor))
      );
  }
  PointF _ZoomedToVirtual(PointF x) const
  {
    return PointF(
      (static_cast<ViewPortSubPixel>(x.x) / zoomFactor),
      (static_cast<ViewPortSubPixel>(x.y) / zoomFactor)
      );
  }

  PointI _VirtualToZoomed(PointI x) const
  {
    /*
      |=============x===========================| zoomed virtual canvas
      |=====x==============| virtual canvas
      <-----> input
      <-------------> return
    */
    return PointI(
      Round(zoomFactor * x.x),
      Round(zoomFactor * x.y)
      );
  }
  PointF _VirtualToZoomed(PointF x) const
  {
    return PointF(
      zoomFactor * x.x,
      zoomFactor * x.y
      );
  }

  // calculate cached members based on the basic members
  void CacheValues()
  {
    zoomedOrigin.x = zoomFactor * virtualOrigin.x;
    zoomedOrigin.y = zoomFactor * virtualOrigin.y;
    /*              origins
                      |
      |===============*============================| zoomed virtual canvas
            |=========*===============|  viewport
      <----> what we are calculating.
      <---------------> zoomedOrigin
            <---------> viewport origin.
    */
    viewOffsetFromZoomed.x = zoomedOrigin.x - viewOrigin.x;
    viewOffsetFromZoomed.y = zoomedOrigin.y - viewOrigin.y;
  }

  // basic members
  PointF viewOrigin;
  PointF virtualOrigin;
  ViewPortSubPixel zoomFactor;

  // cached values
  PointF zoomedOrigin;// always virtualOrigin * zoomFactor
  PointF viewOffsetFromZoomed;// this value + view coords = zoomed coords.  most of the time this should be positive, meaning the view's left edge is INSIDE the virtual canvas.
};


#endif

