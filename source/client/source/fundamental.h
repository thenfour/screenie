// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark


#pragma once


typedef double ViewPortSubPixel;


inline int Round(ViewPortSubPixel x)
{
	if (x >= 0)
		return (int) (x+0.5);
	return (long) (x-0.5);
}

class PointF
{
public:
  typedef PointF This_T;
	typedef ViewPortSubPixel T;

  PointF() :
    x(0),
    y(0)
  {
  }
  PointF(const T& initX, const T& initY) :
    x(initX),
    y(initY)
  {
  }
	explicit PointF(const CPoint& p)
	{
		x = (T)p.x;
		y = (T)p.y;
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
	void Offset(const T& xOffset, const T& yOffset)
  {
    x += xOffset;
    y += yOffset;
  }
	void Assign(const T& X, const T& Y)
  {
    x = X;
    y = Y;
  }
	CPoint Round() const 
	{
		CPoint ret;
		ret.x = static_cast<int>(::Round(x));
		ret.y = static_cast<int>(::Round(y));
		return ret;
	}
	CPoint Floor() const
	{
		CPoint ret;
		ret.x = static_cast<int>(floor(x));
		ret.y = static_cast<int>(floor(y));
		return ret;
	}
	CPoint Ceil() const
	{
		CPoint ret;
		ret.x = static_cast<int>(ceil(x));
		ret.y = static_cast<int>(ceil(y));
		return ret;
	}
	void SelfRound()
	{
		x = ::Round(x);
		y = ::Round(y);
	}
	void SelfFloor()
	{
		x = floor(x);
		y = floor(y);
	}
	void SelfCeil()
	{
		x = ceil(x);
		y = ceil(y);
	}
  T x;
  T y;

	Gdiplus::PointF ToGdiplusPointF()
	{
		return Gdiplus::PointF((Gdiplus::REAL)x, (Gdiplus::REAL)y);
	}

	bool IsEqual(const PointF& rhs, T accuracy) const
	{
		if(abs(x - rhs.x) > accuracy) return false;
		if(abs(y - rhs.y) > accuracy) return false;
		return true;
	}
};

class RectF
{
public:
  typedef RectF This_T;
	typedef ViewPortSubPixel T;

	RectF() :
		left(ul.x),
		top(ul.y),
		right(br.x),
		bottom(br.y)
	{
	}
	RectF(const This_T& rhs) :
		left(ul.x),
		top(ul.y),
		right(br.x),
		bottom(br.y),
		ul(rhs.ul),
		br(rhs.br)
	{
	}
	RectF(const T& left_, const T& top_, const T& right_, const T& bottom_) :
		left(ul.x),
		top(ul.y),
		right(br.x),
		bottom(br.y),
		ul(left_, top_),
		br(right_, bottom_)
	{
	}
	RectF(const PointF& ul_, const PointF& br_) :
		left(ul.x),
		top(ul.y),
		right(br.x),
		bottom(br.y),
		ul(ul_),
		br(br_)
	{
	}
	explicit RectF(const CRect& rc) :
		left(ul.x),
		top(ul.y),
		right(br.x),
		bottom(br.y),
		ul(rc.left, rc.top),
		br(rc.right, rc.bottom)
	{
	}

	void Assign(int l, int t, int r, int b)
	{
		left = l;
		top = t;
		right = r;
		bottom = b;
	}

	This_T& operator =(const This_T& rhs)
	{
		ul = rhs.ul;
		br = rhs.br;
		return *this;
	}

	T Width() const
	{
		return right - left;
	}
	int WidthRounded() const
	{
		return ::Round(right - left);
	}
	T Height() const
	{
		return bottom - top;
	}
	int HeightRounded() const
	{
		return ::Round(bottom - top);
	}
	PointF SizeF() const
	{
		return PointF(Width(), Height());
	}

	void Inflate(const T& v)
	{
		left -= v;
		right += v;
		top -= v;
		bottom += v;
	}
	CRect QuantizeInflate() const
	{
		RectF ret(*this);
		ret.ul.SelfFloor();
		ret.br.SelfCeil();
		return ret.Round();
	}

	bool HitTest(const PointF& p) const
	{
		if(p.x < left) return false;
		if(p.x > right) return false;
		if(p.y < top) return false;
		if(p.y > bottom) return false;
		return true;
	}

	RectF Intersection(const This_T& rhs) const
	{
		// returns the rectangle that both rhs & this cover.
		if(rhs.top >= this->bottom) return RectF(ul, ul);
		if(rhs.left >= this->right) return RectF(ul, ul);
		if(rhs.right < this->left) return RectF(ul, ul);
		if(rhs.bottom < this->top) return RectF(ul, ul);

		RectF ret;
		ret.left = max(rhs.left, this->left);
		ret.right = min(rhs.right, this->right);
		ret.top = max(rhs.top, this->top);
		ret.bottom = min(rhs.bottom, this->bottom);
		return ret;
	}

	RectF Offset(const PointF& d) const
	{
		RectF ret(*this);
		ret.left += d.x;
		ret.right += d.x;
		ret.top += d.y;
		ret.bottom += d.y;
		return ret;
	}

	CRect Round() const
	{
		return CRect(ul.Round(), br.Round());
	}

	T& left;
	T& top;
	T& right;
	T& bottom;

	int LeftRounded() const
	{
		return ::Round(left);
	}

	int TopRounded() const
	{
		return ::Round(top);
	}

	int RightRounded() const
	{
		return ::Round(right);
	}

	int BottomRounded() const
	{
		return ::Round(bottom);
	}

	PointF ul;
	PointF br;
};


// turns a source rect + subtract rect into 4 rects which define
template<typename RectT>
class SubtractRectHelperX
{
public:
	SubtractRectHelperX(const RectT& src, const RectT& subtract)
	{
		// to really understand this you just need to draw a diagram and figure it out.
		top.top = src.top;
		top.bottom = max(src.top, subtract.top);
		top.left = src.left;
		top.right = src.right;

		bottom.top = min(src.bottom, subtract.bottom);
		bottom.bottom = src.bottom;
		bottom.left = top.left;
		bottom.right = top.right;

		left.top = top.bottom;
		left.bottom = bottom.top;// how confusing is all of this?!
		left.left = src.left;
		left.right = max(src.left, subtract.left);

		right.top = top.bottom;
		right.bottom = bottom.top;
		right.left = min(src.right, subtract.right);
		right.right = src.right;
	}

	// resulting rectangles...
	RectT top;
	RectT left;
	RectT right;
	RectT bottom;
};
typedef SubtractRectHelperX<CRect> SubtractRectHelper;
typedef SubtractRectHelperX<RectF> SubtractRectHelperF;

