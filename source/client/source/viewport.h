

#ifndef VIEWPORT_INCLUDED
#define VIEWPORT_INCLUDED

template<typename T>
class Point
{
public:
  typedef Point<T> This_T;

  // Constructors
  Point() :
    x(0),
    y(0)
  {
  }
  Point(T initX, T initY) :
    x(initX),
    y(initY)
  {
  }
  Point(const This_T& rhs) :
    x(rhs.x),
    y(rhs.y)
  {
  }
  template<typename T2>
  Point(const Point<T2>& rhs) :
    x(static_cast<T>(rhs.x)),
    y(static_cast<T>(rhs.y)),
  {
  }

  This_T& operator =(const This_T& rhs)
  {
    x = rhs.x;
    y = rhs.y;
    return *this;
  }

  // Operations
	void Offset(T xOffset, T yOffset)
  {
    x += xOffset;
    y += yOffset;
  }

	void Assign(T X, T Y)
  {
    x = X;
    y = Y;
  }

  T x;
  T y;
};


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
template<typename T>
class Viewport
{
public:
  typedef Point<T> Point_T;
  typedef Viewport<T> This_T;

  Viewport() :
    zoomFactor(1)
  {
  }

  Viewport(const Viewport& rhs) :
    virtualOrigin(rhs.virtualOrigin),
    viewOrigin(rhs.viewOrigin),
    zoomedOrigin(rhs.zoomedOrigin),
    zoomFactor(rhs.zoomFactor),
    viewOffsetFromZoomed(0)
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

  void SetZoomFactor(int z)
  {
    zoomFactor = z;
    CacheValues();
  }
  int GetZoomFactor() const
  {
    return zoomFactor;
  }
  void SetViewOrigin(Point_T o)
  {
    viewOrigin = o;
    CacheValues();
  }
  const Point_T& GetViewOrigin() const
  {
    return viewOrigin;
  }
  void SetVirtualOrigin(Point_T o)
  {
    virtualOrigin = o;
    CacheValues();
  }
  const Point_T& GetVirtualOrigin() const
  {
    return virtualOrigin;
  }

  template<typename T2>
  Point<T2> ViewToVirtual(Point<T2> x)
  {
    return ZoomedToVirtual(ViewToZoomed(x));
  }

  template<typename T2>
  Point<T2> VirtualToView(Point<T2> x)
  {
    return ZoomedToView(VirtualToZoomed(x));
  }

  template<typename T2>
  Point<T2> ViewToVirtualSize(Point<T2> x)
  {
    return ZoomedToVirtualSize(x);
  }

  template<typename T2>
  Point<T2> VirtualToViewSize(Point<T2> x)
  {
    return VirtualToZoomedSize(x);
  }

protected:
  template<typename T2>
  Point<T2> ZoomedToVirtualSize(Point<T2> x)
  {
    return Point<T2>(
      x.x / zoomFactor,
      x.y / zoomFactor
      );
  }

  template<typename T2>
  Point<T2> VirtualToZoomedSize(Point<T2> x)
  {
    return Point<T2>(
      x.x * zoomFactor,
      x.y * zoomFactor
      );
  }

  template<typename T2>
  Point<T2> ViewToZoomed(Point<T2> x)
  {
    /*              origins
                      |
      |===========x===*============================| zoomed virtual canvas
            |=====x===*===============|  viewport
            <-----> input
      <-----------> what we are calculating.
      <----> viewOffsetFromZoomed cached value.
    */
    return Point<T2>(
      x.x + viewOffsetFromZoomed.x,
      x.y + viewOffsetFromZoomed.y
      );
  }

  template<typename T2>
  Point<T2> ZoomedToView(Point<T2> x)
  {
    /*              origins
                      |
      |===========x===*============================| zoomed virtual canvas
            |=====x===*===============|  viewport
      <-----------> input
            <-----> what we are calculating.
      <----> viewOffsetFromZoomed cached value.
    */
    return Point<T2>(
      x.x - viewOffsetFromZoomed.x,
      x.y - viewOffsetFromZoomed.y
      );
  }

  // TODO: consider rounding.
  template<typename T2>
  Point<T2> ZoomedToVirtual(Point<T2> x)
  {
    /*
      |=============x===========================| zoomed virtual canvas
      |=====x==============| virtual canvas
      <-------------> input
      <-----> return
    */
    return Point<T2>(
      x.x / zoomFactor,
      x.y / zoomFactor
      );
  }

  // TODO: consider rounding.
  template<typename T2>
  Point<T2> VirtualToZoomed(Point<T2> x)
  {
    /*
      |=============x===========================| zoomed virtual canvas
      |=====x==============| virtual canvas
      <-----> input
      <-------------> return
    */
    return Point<T2>(
      x.x * zoomFactor,
      x.y * zoomFactor
      );
  }

  // calculate cached members based on the basic members
  void CacheValues()
  {
    zoomedOrigin.x = virtualOrigin.x * zoomFactor;
    zoomedOrigin.y = virtualOrigin.y * zoomFactor;
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
  Point_T viewOrigin;
  Point_T virtualOrigin;
  int zoomFactor;

  // cached values
  Point_T zoomedOrigin;// always virtualOrigin * zoomFactor
  Point_T viewOffsetFromZoomed;// this value + view coords = zoomed coords.  most of the time this should be positive, meaning the view's left edge is INSIDE the virtual canvas.
};


#endif

