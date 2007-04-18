/*
  Jun 05, 2005 carlc - added more rgbpixel helper crap

  Jun 30, 2004 carlc
  This is a class for "animated bitmaps".  Basically the idea is that you get easy low level
  access do a DIB and its meant to be drawn in frames.

  This is only meant for SCREEN purposes, and meant to be FAST!
*/


#pragma once


#include <windows.h>

// turns a source rect + subtract rect into 4 rects which define
class SubtractRectHelper
{
public:
	SubtractRectHelper(const CRect& src, const CRect& subtract)
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
	CRect top;
	CRect left;
	CRect right;
	CRect bottom;
};


// compare 2 floating point numbers to see if they are in a certain range.
template<typename Tl, typename Tr, typename Terr>
inline bool xequals(const Tl& l, const Tr& r, const Terr& e)
{
  Tl d = l - r;
  if(d < 0) d = -d;
  if(d > e) return false;
  return true;
}

template<long bpp>
struct BitmapTypes
{
  typedef unsigned __int8 RgbPixel;
};

template<>
struct BitmapTypes<16>
{
  typedef unsigned __int16 RgbPixel;
};

template<>
struct BitmapTypes<32>
{
  typedef unsigned __int32 RgbPixel;
};

typedef BitmapTypes<32>::RgbPixel RgbPixel32;


inline BYTE R(RgbPixel32 d)
{
  return static_cast<BYTE>((d & 0x00FF0000) >> 16);
}

inline BYTE G(RgbPixel32 d)
{
  return static_cast<BYTE>((d & 0x0000FF00) >> 8);
}

inline BYTE B(RgbPixel32 d)
{
  return static_cast<BYTE>((d & 0x000000FF));
}

inline RgbPixel32 MakeRgbPixel32(BYTE r, BYTE g, BYTE b)
{
  return static_cast<RgbPixel32>((r << 16) | (g << 8) | (b));
}

template<typename T>
inline RgbPixel32 MakeRgbPixel32(const T& r, const T& g, const T& b)
{
  return MakeRgbPixel32(static_cast<BYTE>(r), static_cast<BYTE>(g), static_cast<BYTE>(b));
}

inline COLORREF RgbPixel32ToCOLORREF(RgbPixel32 x)
{
  return RGB(R(x), G(x), B(x));
}

inline RgbPixel32 COLORREFToRgbPixel32(COLORREF x)
{
  return MakeRgbPixel32(GetRValue(x), GetGValue(x), GetBValue(x));
}

inline RgbPixel32 MixColorsInt(long fa, long fmax, RgbPixel32 ca, RgbPixel32 cb)
{
  BYTE r, g, b;
  long fmaxminusfa = fmax - fa;
  r = static_cast<BYTE>(((fa * R(ca)) + (fmaxminusfa * R(cb))) / fmax);
  g = static_cast<BYTE>(((fa * G(ca)) + (fmaxminusfa * G(cb))) / fmax);
  b = static_cast<BYTE>(((fa * B(ca)) + (fmaxminusfa * B(cb))) / fmax);
  return MakeRgbPixel32(r,g,b);
}



template<long Tbpp>
class AnimBitmap
{
public:
  static const long bpp = Tbpp;
  typedef typename BitmapTypes<bpp>::RgbPixel RgbPixel;

	CRect GetArea() const
	{
		return CRect(0, 0, m_x, m_y);
	}

  AnimBitmap() :
    m_y(0),
    m_x(0),
    m_bmp(0),
    m_pbuf(0),
		m_checkerPattern(0),
		m_checkerPatternGrayA(0),
		m_checkerPatternGrayB(0)
  {
    // store our offscreen hdc
    HDC hscreen = ::GetDC(0);
    m_offscreen = CreateCompatibleDC(hscreen);
    ReleaseDC(0, hscreen);
  }

  ~AnimBitmap()
  {
    if(m_bmp)
    {
      SelectObject(m_offscreen, m_oldBitmap);
      DeleteObject(m_bmp);
    }
		if(m_checkerPattern)
		{
			DeleteObject(m_checkerPattern);
			DeleteObject(m_checkerPatternGrayA);
			DeleteObject(m_checkerPatternGrayB);
		}
    DeleteDC(m_offscreen);
  }

  //CSize GetSize() const
  //{
  //  return CSize(m_x, m_y);
  //}

  long GetWidth() const
  {
    return m_x;
  }

  long GetHeight() const
  {
    return m_y;
  }

  // MUST be called at least once.  This will allocate the bmp object.
  bool SetSize(long x, long y)
  {
    bool r = false;

    if(x & 1) x ++;
    if(y & 1) y ++;

    if((x != m_x) || (y != m_y))
    {
      // delete our current bmp.
      if(m_bmp)
      {
        SelectObject(m_offscreen, m_oldBitmap);
        DeleteObject(m_bmp);
        m_bmp = 0;
      }

      long pbiSize = sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 10;
      BITMAPINFO* pbi = reinterpret_cast<BITMAPINFO*>(HeapAlloc(GetProcessHeap(), 0, pbiSize));
      memset(pbi, 0, pbiSize);
      BITMAPINFO& bi = *pbi;

      bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
      bi.bmiHeader.biPlanes = 1;// must be 1

      if(bpp == 32)
      {
        bi.bmiHeader.biBitCount = 32;// for RGBQUAD
        bi.bmiHeader.biCompression = BI_RGB;
      }
      else if(bpp == 16)
      {
        bi.bmiHeader.biBitCount = 16;
        bi.bmiHeader.biCompression = BI_BITFIELDS;
        *(unsigned int*)(&bi.bmiColors[0]) = 0xF800;
        *(unsigned int*)(&bi.bmiColors[1]) = 0x07E0;
        *(unsigned int*)(&bi.bmiColors[2]) = 0x001F;
      }

      bi.bmiHeader.biSizeImage = 0;// dont need to specify because its uncompressed
      bi.bmiHeader.biXPelsPerMeter = 0;
      bi.bmiHeader.biYPelsPerMeter = 0;
      bi.bmiHeader.biClrUsed  = 0;
      bi.bmiHeader.biClrImportant = 0;
      bi.bmiHeader.biWidth = max(x,1);
      bi.bmiHeader.biHeight = -(max(y,1));

      m_bmp = CreateDIBSection(m_offscreen, &bi, DIB_RGB_COLORS, (void**)&m_pbuf, 0, 0);
      bi.bmiHeader.biHeight = (max(y,1));
      if(m_bmp)
      {
        r = true;
        m_x = bi.bmiHeader.biWidth;
        m_y = bi.bmiHeader.biHeight;
        m_oldBitmap = (HBITMAP)SelectObject(m_offscreen, m_bmp);
      }

      HeapFree(GetProcessHeap(), 0, pbi);
    }
    return r;
  }

  // commits the changes
  bool Commit()
  {
    return true;
  }

  bool BeginDraw()
  {
    GdiFlush();
    return true;
  }

  HDC GetDC()
  {
    return m_offscreen;
  }

  // no boundschecking for speed.
  void SetPixel(long x, long y, RgbPixel c)
  {
    ATLASSERT(x >= 0);
    ATLASSERT(y >= 0);
    ATLASSERT(y < m_y);
    ATLASSERT(x < m_x);
    //y = m_y - y;
    m_pbuf[x + (y * m_x)] = c;
  }

  // xright is NOT drawn.
  void SafeHLine(long x1, long x2, long y, RgbPixel c)
  {
    long xleft = min(x1, x2);
    long xright = max(x1, x2);
    if(xleft > 0)
    {
      if(xright <= m_x)
      {
        if(y > 0)
        {
          if(y < m_y)
          {
            HLine(xleft, xright, y, c);
          }
        }
      }
    }
  }

  // xright is NOT drawn.
  void HLine(long x1, long x2, long y, RgbPixel c)
  {
    long xleft = min(x1, x2);
    long xright = max(x1, x2);
    RgbPixel* pbuf = &m_pbuf[(y * m_x) + xleft];
    while(xleft != xright)
    {
      *pbuf = c;
      pbuf ++;
      xleft ++;
    }
  }

  void VLine(long x, long y1, long y2, RgbPixel c)
  {
    long ytop = min(y1, y2);
    long ybottom = max(y1, y2);
    RgbPixel* pbuf = &m_pbuf[(ytop * m_x) + x];
    while(ytop != ybottom)
    {
      *pbuf = c;
      pbuf += m_x;
      ytop ++;
    }
  }

  // b and r are not drawn
  void Rect(long l, long t, long r, long b, RgbPixel c)
  {
    RgbPixel* pbuf = &m_pbuf[(t * m_x) + l];
    long h = r - l;// horizontal size
    // fill downwards
    while(t != b)
    {
      // draw a horizontal line
      for(long i = 0; i < h; i ++)
      {
        pbuf[i] = c;
      }
      pbuf += m_x;
      t ++;
    }
  }

  // b and r are not drawn
  void SafeRect(long l, long t, long r, long b, RgbPixel c)
  {
    clamp(l, 0, m_x);
    clamp(r, 0, m_x);
    clamp(t, 0, m_y);
    clamp(b, 0, m_y);
    Rect(l, t, r, b, c);
  }

  bool SetPixelSafe(long x, long y, RgbPixel c)
  {
    bool r = false;
    if(x > 0 && y > 0 && x < m_x && y < m_y)
    {
      m_pbuf[x + (y * m_x)] = c;
      r = true;
    }
    return r;
  }

  RgbPixel GetPixel(long x, long y)
  {
    return m_pbuf[x + (y * m_x)];
  }

  bool GetPixelSafe(RgbPixel& out, long x, long y)
  {
    bool r = false;
    if(x > 0 && y > 0 && x < m_x && y < m_y)
    {
      out = m_pbuf[x + (y * m_x)];
      r = true;
    }
    return r;
  }

  void Fill(RgbPixel c)
  {
    RgbPixel* pDest = m_pbuf;
    long n = m_x * m_y;
    for(long i = 0; i < n; i ++)
    {
      pDest[i] = c;
    }
  }

	HBRUSH _CreatePatternBrush(HBITMAP src, int srcwidth, int srcheight, int destwidth, int destheight)
	{
		// god this is annoying all this work just to 1) create a bitmap, 2) patblt, 3) createpatternbrush
		HBRUSH tbr = CreatePatternBrush(src);
		HBITMAP tbmp = CreateCompatibleBitmap(m_offscreen, destwidth, destheight);
		HDC dc2 = CreateCompatibleDC(m_offscreen);

		HBITMAP bmpOld = (HBITMAP)SelectObject(dc2, tbmp);
		HBRUSH brOld = (HBRUSH)SelectObject(dc2, tbr);

		::PatBlt(dc2, 0, 0, destwidth, destheight, PATCOPY);

		SelectObject(dc2, brOld);
		SelectObject(dc2, bmpOld);

		HBRUSH ret = CreatePatternBrush(tbmp);

		DeleteDC(dc2);
		DeleteObject(tbmp);
		DeleteObject(tbr);

		return ret;
	}

	void CacheCheckerPattern()
	{
		if(m_checkerPattern == 0)
		{
			HBITMAP normal = 0;
			HBITMAP grayA = 0;
			HBITMAP grayB = 0;
			const int size = 8;
			const int cacheSize = 256;// size of the cached pattern.

			{
				HBRUSH h1 = CreateSolidBrush(RGB(204, 204, 204));
				HBRUSH h2 = CreateSolidBrush(RGB(255, 255, 255));

				HBITMAP hbm = CreateCompatibleBitmap(m_offscreen, size * 2, size * 2);
				HDC dc = CreateCompatibleDC(m_offscreen);
				
				HGDIOBJ hOld = SelectObject(dc, hbm);
				// generate this pattern.
				CRect rc(0, 0, size * 2, size * 2);
				FillRect(dc, &rc, h1);
				rc.SetRect(0, 0, size, size);
				FillRect(dc, &rc, h2);
				rc.SetRect(size, size, size*2, size*2);
				FillRect(dc, &rc, h2);

				m_checkerPattern = _CreatePatternBrush(hbm, size * 2, size * 2, cacheSize, cacheSize);

				SelectObject(dc, hOld);
				DeleteDC(dc);
				DeleteObject(hbm);
				DeleteObject(h2);
				DeleteObject(h1);
			}
			{
				HBRUSH h1 = CreateSolidBrush(RGB(255, 204, 204));
				HBRUSH h2 = CreateSolidBrush(RGB(0, 255, 255));

				HBITMAP hbm = CreateCompatibleBitmap(m_offscreen, size * 2, size * 2);
				HDC dc = CreateCompatibleDC(m_offscreen);
				
				HGDIOBJ hOld = SelectObject(dc, hbm);
				// generate this pattern.
				CRect rc(0, 0, size * 2, size * 2);
				FillRect(dc, &rc, h1);
				rc.SetRect(0, 0, size, size);
				FillRect(dc, &rc, h2);
				rc.SetRect(size, size, size*2, size*2);
				FillRect(dc, &rc, h2);

				m_checkerPatternGrayA = _CreatePatternBrush(hbm, size * 2, size * 2, cacheSize, cacheSize);

				SelectObject(dc, hOld);
				DeleteDC(dc);
				DeleteObject(hbm);
				DeleteObject(h2);
				DeleteObject(h1);
			}
			{
				HBRUSH h1 = CreateSolidBrush(RGB(204, 255, 204));
				HBRUSH h2 = CreateSolidBrush(RGB(255, 0, 255));

				HBITMAP hbm = CreateCompatibleBitmap(m_offscreen, size * 2, size * 2);
				HDC dc = CreateCompatibleDC(m_offscreen);
				
				HGDIOBJ hOld = SelectObject(dc, hbm);
				// generate this pattern.
				CRect rc(0, 0, size * 2, size * 2);
				FillRect(dc, &rc, h1);
				rc.SetRect(0, 0, size, size);
				FillRect(dc, &rc, h2);
				rc.SetRect(size, size, size*2, size*2);
				FillRect(dc, &rc, h2);

				m_checkerPatternGrayB = _CreatePatternBrush(hbm, size * 2, size * 2, cacheSize, cacheSize);

				SelectObject(dc, hOld);
				DeleteDC(dc);
				DeleteObject(hbm);
				DeleteObject(h2);
				DeleteObject(h1);
			}
		}
	}
	void _PatBlt(const RECT& rc)
	{
		PatBlt(m_offscreen, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, PATCOPY);
	}
	void FillCheckerPatternExclusion(const CRect& exclusionArea, bool grayed)
	{
		CacheCheckerPattern();
		HGDIOBJ hOld;
		if(!grayed)
		{
			hOld = SelectObject(m_offscreen, m_checkerPattern);
		}
		else
		{
			if((exclusionArea.left & 1) == (exclusionArea.top & 1))// determine if we should gray out the even or odd pattern. this sorta goes against the blackbox nature of this, but it's easy so meh.
			{
				hOld = SelectObject(m_offscreen, m_checkerPatternGrayA);
			}
			else
			{
				hOld = SelectObject(m_offscreen, m_checkerPatternGrayB);
			}
		}
		SubtractRectHelper s(CRect(0, 0, m_x, m_y), exclusionArea);
		_PatBlt(s.top);
		_PatBlt(s.left);
		_PatBlt(s.right);
		_PatBlt(s.bottom);
		SelectObject(m_offscreen, hOld);
	}
	//void FillCheckerPattern(const CRect& rc)
	//{
	//	CacheCheckerPattern();
	//	HGDIOBJ hOld = SelectObject(m_offscreen, m_checkerPattern);
	//	_PatBlt(rc);
	//	SelectObject(m_offscreen, hOld);
	//}
	//void FillCheckerPattern()
	//{
	//	CacheCheckerPattern();
	//	HGDIOBJ hOld = SelectObject(m_offscreen, m_checkerPattern);
	//	PatBlt(m_offscreen, 0, 0, m_x, m_y, PATCOPY);
	//	SelectObject(m_offscreen, hOld);
	//}
	//void FillCheckerPatternGrayA()
	//{
	//	CacheCheckerPattern();
	//	HGDIOBJ hOld = SelectObject(m_offscreen, m_checkerPatternGrayA);
	//	PatBlt(m_offscreen, 0, 0, m_x, m_y, PATCOPY);
	//	SelectObject(m_offscreen, hOld);
	//}
	//void FillCheckerPatternGrayB()
	//{
	//	CacheCheckerPattern();
	//	HGDIOBJ hOld = SelectObject(m_offscreen, m_checkerPatternGrayB);
	//	PatBlt(m_offscreen, 0, 0, m_x, m_y, PATCOPY);
	//	SelectObject(m_offscreen, hOld);
	//}

  inline static RgbPixel32 InvertColorForSelection(const RgbPixel32& x, int offset)
  {
    return MakeRgbPixel32(
      R(x) + offset,
      G(x) + offset * 2,
      B(x) + offset * 3
      );
  }

  // "skip" lets us make this for either vertical or horizontal lines.  skip=1 = horizontal.  skip=m_x = vertical.
  template<int patternFreq, int colorOffset>// templated for efficiency.
  void DrawSelectionWithOffset(int patternOffset, RgbPixel32* startPixel, RgbPixel32* endPixel, int skip)
  {
    const int tickSize = patternFreq / 2;
    RgbPixel32* start = startPixel + (patternOffset*skip);
    RgbPixel32* p = start;
    int totalPixels = (endPixel - startPixel) / skip;
    int count = 0;
    int i;
    for(;;)
    {
      for(i = 0; i < tickSize; i ++, count ++, p += skip)
      {
        if(count >= totalPixels) return;
        if(p >= endPixel) p = startPixel;
        *p = InvertColorForSelection(*p, colorOffset);
      }
      for(i = 0; i < tickSize; i ++, count ++, p += skip)
      {
        if(count >= totalPixels) return;
        if(p >= endPixel) p = startPixel;
        *p = InvertColorForSelection(*p, -colorOffset);
      }
    }
  }

  template<int patternFreq, int colorOffset>// templated for efficiency.
  void DrawSelectionHLineSafe(int patternOffset, int y, int xleft, int xright)
  {
    if(y < 0) return;
    if(y >= m_y) return;
    if(xleft < 0) xleft = 0;
    if(xright >= m_x) xright = m_x - 1;
    RgbPixel32* leftMostPixel = &m_pbuf[(y * m_x) + xleft];
    RgbPixel32* rightMostPixel = leftMostPixel + xright - xleft;
    DrawSelectionWithOffset<patternFreq, colorOffset>(patternOffset, leftMostPixel, rightMostPixel, 1);
  }

  template<int patternFreq, int colorOffset>// templated for efficiency.
  void DrawSelectionVLineSafe(int patternOffset, int x, int ytop, int ybottom)
  {
    if(x < 0) return;
    if(x >= m_x) return;
    if(ytop < 0) ytop = 0;
    if(ybottom >= m_y) ybottom = m_y - 1;
    RgbPixel32* topMostPixel = &m_pbuf[(ytop * m_x) + x];
    RgbPixel32* bottomMostPixel = topMostPixel + ((ybottom - ytop) * m_x);
    DrawSelectionWithOffset<patternFreq, colorOffset>(patternOffset, topMostPixel, bottomMostPixel, m_x);
  }

  enum SelectionRectFlags
  {
    SR_DEFAULT = 0,
    SR_IGNORELEFT = 1,
    SR_IGNORERIGHT = 2,
    SR_IGNORETOP = 4,
    SR_IGNOREBOTTOM = 8
  };

	template<int factor>
	inline static RgbPixel GrayPixel(RgbPixel c)
	{
		DWORD x = (R(c) + G(c) + B(c)) >> factor;// grays it & dims it at the same time.
		return MakeRgbPixel32(x,x,x);
	}

	void GrayOut()
	{
		RgbPixel* i = m_pbuf;
		RgbPixel* end = m_pbuf + (m_y * m_x);
		int x = 0;
		int y = 0;
		for(; i < end; i ++)
		{
			if(y & (x & 1))
			{
				*i = GrayPixel<2>(*i);
			}
			else
			{
				*i = GrayPixel<4>(*i);
			}
			x ++;
			if(x == m_x)
			{
				x = 0;
				y ++;
			}
		}
	}

	void GrayRect(int left, int top, int right, int bottom)
	{
    clamp(left, 0, m_x);
    clamp(right, 0, m_x);
    clamp(top, 0, m_y);
    clamp(bottom, 0, m_y);

		RgbPixel* pbuf = &m_pbuf[(top * m_x) + left];
    long width = right - left;

		int y = top;
		// fill downwards
    while(y < bottom)
    {
			int x = left;
      // draw a horizontal line
      for(long tx = 0; tx < width; tx ++)
      {
				if(y & (x & 1))
				{
					pbuf[tx] = GrayPixel<4>(pbuf[tx]);
				}
				else
				{
					pbuf[tx] = GrayPixel<2>(pbuf[tx]);
				}
				x ++;
      }
			pbuf += m_x;
      y ++;
    }
	}

	void GrayRect(const CRect& rc)
	{
		GrayRect(rc.left, rc.top, rc.right, rc.bottom);
	}

	// faster version of GrayRect that works on entire scanlines.
	//void GrayRasters(int top, int bottom)
	//{
 //   clamp(top, 0, m_y);
 //   clamp(bottom, 0, m_y);

	//	RgbPixel* start = m_pbuf + (top * m_x);
	//	RgbPixel* end = m_pbuf + (bottom * m_x);

	//	int y = top;
	//	int x = 0;

	//	for(; start < end; start ++)
	//	{
	//		if(y & (x & 1))
	//		{
	//			*start = GrayPixel<3>(*start);
	//		}
	//		else
	//		{
	//			*start = GrayPixel<4>(*start);
	//		}
	//		x ++;
	//		if(x == m_x)
	//		{
	//			y ++;
	//			x = 0;
	//		}
	//	}
	//}

  template<int patternFreq, int colorOffset, bool GrayBorder, bool showAnts>
	void DrawSelectionRectSafe(int patternOffset, RECT& rc, SelectionRectFlags f = SR_DEFAULT)
  {
    // the formula for color is inverted, plus or minus 20 (with wrap) on each colorant.
		if(showAnts)
		{
			if(!(f & SR_IGNORETOP) && (rc.top > 0) && (rc.top < m_y))
			{
				DrawSelectionHLineSafe<patternFreq, colorOffset>(patternOffset, rc.top, rc.left, rc.right);
			}
			if(!(f & SR_IGNOREBOTTOM) && (rc.bottom > 0) && (rc.bottom < m_y))
			{
				DrawSelectionHLineSafe<patternFreq, colorOffset>(patternFreq - patternOffset, rc.bottom - 1, rc.left, rc.right + 1);
			}
			if(!(f & SR_IGNORELEFT) && (rc.left > 0) && (rc.left < m_x))
			{
				DrawSelectionVLineSafe<patternFreq, colorOffset>(patternFreq - patternOffset, rc.left, rc.top, rc.bottom);
			}
			if(!(f & SR_IGNORERIGHT) && (rc.right > 0) && (rc.right < m_x))
			{
				DrawSelectionVLineSafe<patternFreq, colorOffset>(patternOffset, rc.right - 1, rc.top, rc.bottom);
			}
		}

		if(GrayBorder)
		{
			GrayRasters(0, rc.top);// top
			GrayRasters(rc.bottom, m_y);// bottom
			GrayRect(0, rc.top, rc.left, rc.bottom);// left
			GrayRect(rc.right, rc.top, m_x, rc.bottom);// right
		}
  }

  bool StretchBlit(AnimBitmap& dest, long destx, long desty, long destw, long desth, long srcx, long srcy, long srcw, long srch, int mode = HALFTONE)
  {
    SetStretchBltMode(dest.m_offscreen, mode);
    int r = StretchBlt(
      dest.m_offscreen, destx, desty, destw, desth,
      m_offscreen, srcx, srcy, srcw, srch, SRCCOPY);
    return r != 0;
  }

  bool StretchBlit(AnimBitmap& dest, long x, long y, long w, long h, int mode = HALFTONE)
  {
    SetStretchBltMode(dest.m_offscreen, mode);
    int r = StretchBlt(dest.m_offscreen, x, y, w, h, m_offscreen, 0, 0, m_x, m_y, SRCCOPY);
    return r != 0;
  }

  bool StretchBlit(HDC hDest, long x, long y, long w, long h)
  {
    int r = StretchBlt(hDest, x, y, w, h, m_offscreen, 0, 0, m_x, m_y, SRCCOPY);
    return r != 0;
  }

  bool Blit(HDC hDest, long x, long y)
  {
    int r = BitBlt(hDest, x, y, x + m_x, y + m_y, m_offscreen, 0, 0, SRCCOPY);
    return r != 0;
  }

  bool Blit(HDC hDest, long destX, long destY, long width, long height, long srcX = 0, long srcY = 0)
  {
    int r = BitBlt(hDest, destX, destY, destX + width, destY + height, m_offscreen, srcX, srcY, SRCCOPY);
    return r != 0;
  }

  template<long _Tbpp>
  bool Blit(AnimBitmap<_Tbpp>& dest, long x, long y)
  {
    int r = BitBlt(dest.m_offscreen, x, y, x + m_x, y + m_y, m_offscreen, 0, 0, SRCCOPY);
    return r != 0;
  }

  // copies directly from rc to rc.
  template<long _Tbpp>
  bool Blit(AnimBitmap<_Tbpp>& dest, const RECT& rc)
  {
    int r = BitBlt(dest.m_offscreen,
		rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 
		m_offscreen,
		rc.left, rc.top, SRCCOPY);
	return r != 0;
  }

	  // blits to a similar-sized destination, MINUS some rectangle
  template<long _Tbpp>
  void ExclusionBlit(AnimBitmap<_Tbpp>& dest, const RECT& rcSubtraction)
  {
	  SubtractRectHelper s(CRect(0, 0, m_x, m_y), rcSubtraction);
	  Blit(dest, s.top);
	  Blit(dest, s.left);
	  Blit(dest, s.right);
	  Blit(dest, s.bottom);
  }

  template<long _Tbpp>
  bool Blit(AnimBitmap<_Tbpp>& dest, long destx, long desty, long width, long height, long srcx = 0, long srcy = 0)
  {
    int r = BitBlt(dest.m_offscreen, destx, desty, width, height, m_offscreen, srcx, srcy, SRCCOPY);
    return r != 0;
  }

  bool BlitFrom(HDC src, long x, long y, long w, long h, long DestX = 0, long DestY = 0)
  {
    int r = BitBlt(m_offscreen, DestX, DestY, w, h, src, x, y, SRCCOPY);
    return r != 0;
  }

  bool BlitFrom(HBITMAP src, long x, long y, long w, long h, long DestX = 0, long DestY = 0)
  {
    HDC dcScreen = ::GetDC(0);
    HDC dcMem = CreateCompatibleDC(dcScreen);
    ReleaseDC(0, dcScreen);
    HBITMAP hOld = (HBITMAP)SelectObject(dcMem, src);
    int r = BitBlt(m_offscreen, DestX, DestY, w, h, dcMem, x, y, SRCCOPY);
    SelectObject(dcMem, hOld);
    DeleteDC(dcMem);
    return r != 0;
  }

  bool StretchBlitFrom(int destx, int desty, int destw, int desth, HDC src, int srcx, int srcy, int srcw, int srch, int mode = HALFTONE)
  {
    SetStretchBltMode(m_offscreen, mode);
    int r = StretchBlt(m_offscreen, destx, desty, destw, desth, src, srcx, srcy, srcw, srch, SRCCOPY);
    return r != 0;
  }

  bool _DrawText(const char* s, long x, long y)
  {
    RECT rc;
    rc.left = x;
    rc.top = y;
    rc.right = m_x;
    rc.bottom = m_y;
    DrawText(m_offscreen, s, static_cast<int>(strlen(s)), &rc, DT_NOCLIP);
    return true;
  }

  RgbPixel* GetBuffer()
  {
    return m_pbuf;
  }

  HBITMAP DetachHandle()
  {
    HBITMAP ret = m_bmp;
    if(m_bmp)
    {
      SelectObject(m_offscreen, m_oldBitmap);
      m_bmp = 0;
      DeleteDC(m_offscreen);
    }
    return ret;
  }
	HBITMAP GetHBITMAP()
	{
		return m_bmp;
	}

private:
  long m_x;
  long m_y;
  HDC m_offscreen;
  HBITMAP m_bmp;
  HBITMAP m_oldBitmap;
  RgbPixel* m_pbuf;

	HBRUSH m_checkerPattern;
	HBRUSH m_checkerPatternGrayA;
	HBRUSH m_checkerPatternGrayB;

  template<typename T, typename Tmin, typename Tmax>
  inline bool clamp(T& l, const Tmin& minval, const Tmax& maxval)
  {
    if(l < static_cast<T>(minval))
    {
      l = static_cast<T>(minval);
      return true;
    }
    if(l > static_cast<T>(maxval))
    {
      l = static_cast<T>(maxval);
      return true;
    }
    return false;
  }
};

