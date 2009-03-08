// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark

/*
  Jun 5, 2005 - updated for latest libcc blob

  todo:
  -------------------------------------------
        -) support odd diameters
        -) write line functions
        -) write thickline functions
*/


#pragma once


// stores heights of a circle, and can repeat them out for any X.
class CircleHeights
{
public:
  typedef unsigned short Height_T;

  template<typename T>
  void Init(T radius)
  {
    m_rad = static_cast<Height_T>(radius);
    //radius -= 1;
    long d = 3 - (2 * radius);
    long x = 0;
    long y = radius;
    m_buf.Realloc(radius+1);
    Height_T* pStart = m_buf.GetLockedBuffer();
    Height_T* pEnd = pStart + radius - 1;
    *pStart = static_cast<Height_T>(radius);

    while(x <= y)
    {
      *pStart = static_cast<Height_T>(y-1);

      if(d < 0)
      {
        d += (4 * x) + 6;
      }
      else
      {
        *pEnd = static_cast<Height_T>(x);
        *pEnd --;
        y --;
        d += 4 * (x - y) + 10;
      }

      x ++;
      pStart ++;
    }

    m_45 = static_cast<Height_T>(x);
    return;
  }

  // no bound checking for optimization
  template<typename T> inline Height_T GetHeight(T x) const { return m_buf.GetLockedBuffer()[static_cast<Height_T>(x)]; }
  inline Height_T Get45Mark() const { return m_45; }
  inline Height_T GetRadius() const { return m_rad; }
private:
  Height_T m_45;// at what X does the 45 degree point hit?
  Height_T m_rad;
  LibCC::Blob<Height_T, LibCC::BlobTraits<false, 1> > m_buf;
};


// stores heights AND outer-pixel-antialias values of a circle for any given X.
// aa values are only calculated until the 45 mark (after that the height starts decrementing
// by more than 1 and antialiasing would look dumb if you dont replicate over octants like a
// good boy)
template<bool bInner = false>
class CircleHeightsAA
{
public:
  typedef unsigned short Height_T;

  template<typename T>
  void Init(T radius)
  {
    long hMinus1;
    long hMinus1Squared;
    long hPlus1;
    long hPlus1Squared;
    long delta;// used for AA
    Height_T* pAA;

    m_rad = static_cast<Height_T>(radius);
    long x = 0;
    long x2 = 0;// x squared
    long r2 = radius * radius;
    long h = radius;

    m_heights.Alloc(radius+2);
    m_aavalues.Alloc(radius);
    pAA = m_aavalues.GetBuffer();

    Height_T* pStart = m_heights.GetBuffer();
    Height_T* pEnd = pStart + radius;

    hPlus1 = h + 1;
    hPlus1Squared = hPlus1 * hPlus1;

    if(bInner)
    {
      hMinus1 = h;// because we changed the meaning of this circle to "subtracted", this is the SAME as h, not minus 1.
      hMinus1Squared = hMinus1 * hMinus1;
      m_aamax = static_cast<Height_T>(((radius + 1) * ( radius + 1)) - r2);
    }
    else
    {
      m_aamax = static_cast<Height_T>(r2 - ((radius - 1) * (radius - 1)));
    }

    while(x <= h)
    {
      if(bInner)
      {
        // we want the first point thats OUTSIDE the circle.
        if((x2 + hMinus1Squared - r2) > 0)
        {
          *pEnd = static_cast<Height_T>(x - 1);
          *pEnd --;
          h --;
          hMinus1 --;
          hMinus1Squared = hMinus1 * hMinus1;
        }

        delta = r2 - (hMinus1Squared + x2);
        delta = m_aamax - delta;//because we flip-flop the background/foreground colors for the inside 
      }
      else
      {
        // decrement height until its inside the circle
        if((x2 + hPlus1Squared - r2) >= 0)
        {
          *pEnd = static_cast<Height_T>(x - 1);
          *pEnd --;
          h --;
          hPlus1 --;
          hPlus1Squared = hPlus1 * hPlus1;
        }

        delta = r2 - (hPlus1Squared + x2);
      }

      if(delta < 0) delta = 0;
      if(delta > m_aamax) delta = m_aamax;
      *pAA = static_cast<Height_T>(delta);
      pAA ++;

      *pStart = static_cast<Height_T>(h);
      pStart ++;

      x++;
      x2 = x*x;
    }

    m_45 = static_cast<Height_T>(x);
    return;
  }

  // no bound checking for optimization
  template<typename T> inline Height_T GetHeight(T x) const { return m_heights.GetBuffer()[static_cast<Height_T>(x)]; }
  template<typename T> inline Height_T GetAAValue(T x) const { return m_aavalues.GetBuffer()[static_cast<Height_T>(x)]; }
  inline Height_T Get45Mark() const { return m_45; }
  inline Height_T GetRadius() const { return m_rad; }
  inline Height_T GetAAMax() const { return m_aamax; }
private:
  Height_T m_45;// at what X does the 45 degree point hit?
  Height_T m_rad;
  Height_T m_aamax;// maximum number for antialias values.
  LibCC::Blob<Height_T, LibCC::BlobTraits<false, 1> > m_heights;
  LibCC::Blob<Height_T, LibCC::BlobTraits<false, 1> > m_aavalues;
};




template<typename Tsh, typename Tshproc, typename Ta, typename Taproc>
void FilledCircleAAG(long cx, long cy, long r, Tsh sh, Tshproc shproc, Ta a, Taproc aproc)
{
  CircleHeightsAA<false> heights;
  heights.Init(r);
  CircleHeightsAA<false>::Height_T h;

  if(r)
  {
    for(long y = 0; y < r; ++ y)
    {
      h = heights.GetHeight(y);
      (sh->*shproc)(cx - h - 1, cx + h, cy + y);
      (sh->*shproc)(cx - h - 1, cx + h, cy - y - 1);
    }

    for(long y = 0; y < heights.Get45Mark(); ++ y)
    {
      h = heights.GetHeight(y);
      (a->*aproc)(cx, cy, h + 1, y, heights.GetAAValue(y), heights.GetAAMax());
      (a->*aproc)(cx, cy, y, h + 1, heights.GetAAValue(y), heights.GetAAMax());
    }
  }
}


// not antialiased.  based on bresenham.  all horizontal lines just like above.
template<typename Tsh, typename Tshproc>
void FilledCircleG(long cx, long cy, long r, Tsh sh, Tshproc shproc)
{
  CircleHeights heights;
  heights.Init(r);
  CircleHeights::Height_T h;

  for(long y = 0; y < r; ++ y)
  {
    h = heights.GetHeight(y);
    (sh->*shproc)(cx - h - 1, cx + h, cy + y);
    (sh->*shproc)(cx - h - 1, cx + h, cy - y - 1);
  }

  return;
}


template<typename Th, typename Thproc>
void DonutG(long cx, long cy, long rin, long width, Th h, Thproc hproc)
{
  CircleHeights outer;
  CircleHeights inner;

  outer.Init(rin+width);
  inner.Init(rin);

  long y;
  CircleHeights::Height_T hOuter;
  CircleHeights::Height_T hInner;

  if(r)
  {
    for(y = 0; y < inner.GetRadius(); y ++)
    {
      hOuter = outer.GetHeight(y);
      hInner = inner.GetHeight(y);
      // exclude the inner circle
      (h->*hproc)(cx + hInner + 1, cx + hOuter, cy + y);
      (h->*hproc)(cx + hInner + 1, cx + hOuter, cy - y - 1);
      (h->*hproc)(cx - 1 - hOuter, cx - hInner - 2, cy + y);
      (h->*hproc)(cx - 1 - hOuter, cx - hInner - 2, cy - y - 1);
    }

    for(; y < outer.GetRadius(); ++ y)
    {
      hOuter = outer.GetHeight(y);
      (h->*hproc)(cx - 1 - hOuter, cx + hOuter, cy + y);
      (h->*hproc)(cx - 1 - hOuter, cx + hOuter, cy - y - 1);
    }
  }
  return;
}


template<typename Th, typename Thproc, typename Ta, typename Taproc>
void DonutAAG(long cx, long cy, long rin, long width, Th h, Thproc hproc, Ta a, Taproc aproc)
{
  CircleHeightsAA<false> outer;
  CircleHeightsAA<true> inner;

  outer.Init(rin+width);
  inner.Init(rin);

  long y;
  CircleHeightsAA<true>::Height_T hOuter;
  CircleHeightsAA<true>::Height_T hInner;
  CircleHeightsAA<true>::Height_T aaOuter;
  CircleHeightsAA<true>::Height_T aaInner;

  for(y = 0; y < rin; y ++)
  {
    hOuter = outer.GetHeight(y);
    hInner = inner.GetHeight(y);

    (h->*hproc)(cx + hInner + 1, cx + hOuter, cy + y);
    (h->*hproc)(cx + hInner + 1, cx + hOuter, cy - y - 1);
    (h->*hproc)(cx - hOuter - 1, cx - hInner - 2, cy + y);
    (h->*hproc)(cx - hOuter - 1, cx - hInner - 2, cy - y - 1);
  }

  for(; y < outer.GetRadius(); ++ y)
  {
    hOuter = outer.GetHeight(y);
    (h->*hproc)(cx - hOuter - 1, cx + hOuter, cy + y);
    (h->*hproc)(cx - hOuter - 1, cx + hOuter, cy - y - 1);
  }

  // do all antialias
  for(y = 0; y < inner.Get45Mark(); y ++)
  {
    hInner = inner.GetHeight(y);
    aaInner = inner.GetAAValue(y);
    (a->*aproc)(cx, cy, hInner, y, aaInner, inner.GetAAMax());
    (a->*aproc)(cx, cy, y, hInner, aaInner, inner.GetAAMax());
  }

  for(y = 0; y < outer.Get45Mark(); y ++)
  {
    hOuter = outer.GetHeight(y);
    aaOuter = outer.GetAAValue(y);
    (a->*aproc)(cx, cy, hOuter + 1, y, aaOuter, outer.GetAAMax());
    (a->*aproc)(cx, cy, y, hOuter + 1, aaOuter, outer.GetAAMax());
  }

  return;
}

