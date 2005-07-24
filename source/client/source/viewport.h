

#ifndef VIEWPORT_INCLUDED
#define VIEWPORT_INCLUDED


inline int Round(float f)
{
  return static_cast<int>(f + 0.5f);
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
  PointF(float initX, float initY) :
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
	void Offset(float xOffset, float yOffset)
  {
    x += xOffset;
    y += yOffset;
  }

	void Assign(float X, float Y)
  {
    x = X;
    y = Y;
  }

  float x;
  float y;
};

inline PointI PointFtoI(PointF x)
{
  return PointI(Round(x.x), Round(x.y));
}

inline PointF PointItoF(PointI x)
{
  return PointF(static_cast<float>(x.x), static_cast<float>(x.y));
}


/*
  Class to encapsulate translating between a "virtual canvas" and a small view into it (like a huge image & the window that displays part of it).
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

  void SetZoomFactor(float z)
  {
    zoomFactor = z;
    CacheValues();
  }
  float GetZoomFactor() const
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
    return ZoomedToVirtual(ViewToZoomed(x));
  }

  PointF ViewToVirtual(PointF x) const
  {
    return ZoomedToVirtual(ViewToZoomed(x));
  }

  PointI VirtualToView(PointI x) const
  {
    return ZoomedToView(VirtualToZoomed(x));
  }

  PointF VirtualToView(PointF x) const
  {
    return ZoomedToView(VirtualToZoomed(x));
  }

  PointI ViewToVirtualSize(PointI x) const
  {
    return ZoomedToVirtualSize(x);
  }

  PointF ViewToVirtualSize(PointF x) const
  {
    return ZoomedToVirtualSize(x);
  }

  PointI VirtualToViewSize(PointI x) const
  {
    return VirtualToZoomedSize(x);
  }

  PointF VirtualToViewSize(PointF x) const
  {
    return VirtualToZoomedSize(x);
  }

protected:
  PointI ZoomedToVirtualSize(PointI x) const
  {
    return PointI(
      Round((static_cast<float>(x.x) / zoomFactor)),
      Round((static_cast<float>(x.y) / zoomFactor))
      );
  }
  PointF ZoomedToVirtualSize(PointF x) const
  {
    return PointF(
      (static_cast<float>(x.x) / zoomFactor),
      (static_cast<float>(x.y) / zoomFactor)
      );
  }

  PointI VirtualToZoomedSize(PointI x) const
  {
    return PointI(
      Round(zoomFactor * x.x),
      Round(zoomFactor * x.y)
      );
  }

  PointF VirtualToZoomedSize(PointF x) const
  {
    return PointF(
      zoomFactor * x.x,
      zoomFactor * x.y
      );
  }

  PointI ViewToZoomed(PointI x) const
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

  PointF ViewToZoomed(PointF x) const
  {
    return PointF(
      viewOffsetFromZoomed.x + x.x,
      viewOffsetFromZoomed.y + x.y
      );
  }

  PointI ZoomedToView(PointI x) const
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
  PointF ZoomedToView(PointF x) const
  {
    return PointF(
      x.x - viewOffsetFromZoomed.x,
      x.y - viewOffsetFromZoomed.y
      );
  }

  PointI ZoomedToVirtual(PointI x) const
  {
    /*
      |=============x===========================| zoomed virtual canvas
      |=====x==============| virtual canvas
      <-------------> input
      <-----> return
    */
    return PointI(
      Round((static_cast<float>(x.x) / zoomFactor)),
      Round((static_cast<float>(x.y) / zoomFactor))
      );
  }
  PointF ZoomedToVirtual(PointF x) const
  {
    return PointF(
      (static_cast<float>(x.x) / zoomFactor),
      (static_cast<float>(x.y) / zoomFactor)
      );
  }

  PointI VirtualToZoomed(PointI x) const
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
  PointF VirtualToZoomed(PointF x) const
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
  float zoomFactor;

  // cached values
  PointF zoomedOrigin;// always virtualOrigin * zoomFactor
  PointF viewOffsetFromZoomed;// this value + view coords = zoomed coords.  most of the time this should be positive, meaning the view's left edge is INSIDE the virtual canvas.
};


#endif

