// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark

// lookup tables for polar coordinate conversion.
// updated June 6, 2005 carlc
//  - added GetRadius()
//  - fitted new LibCC blob


#pragma once


inline float GetPI()
{
  return (3.1416f);
}

/*
  the lookup table holds values for -r,-r to r,r
*/
template<typename Tr>
class AngleLut
{
public:
  AngleLut() :
    m_radius(0)
  {
  }

  unsigned long GetRadius() const
  {
    return m_radius;
  }

  bool Resize(long radius, const Tr& maxval, const Tr& rotation)
  {
    radius = abs(radius);
    bool r = false;
    if((radius <= static_cast<long>(m_radius)) && (xequals(m_maxval, maxval, 0.01)) && (xequals(m_rotation, rotation, 0.01)))
    {
      r = true;
    }
    else
    {
      m_maxval = maxval;
      m_rotation = rotation;

      unsigned long diam = radius + radius;
      if((diam*diam) < 25000000)
      {
        m_radius = radius;
        m_pitch = diam;

        if(m_dat.Alloc(diam*diam))
        {
          r = true;
          // the origin will be radius down + radius right
          m_origin = m_dat.GetBuffer() + (radius*diam) + radius;

          long sx, sy;
          Tr maxval2 = maxval / 2;
          for(sy = 0; sy < radius; sy ++)
          {
            for(sx = 0; sx < radius; sx ++)
            {
              // the 4 quadrants
              _GetIndex(sx, sy)[0] = GetAdHocAngle(sx, sy, maxval, rotation);
              _GetIndex(-sx, sy)[0] = GetAdHocAngle(-sx, sy, maxval, rotation);
              _GetIndex(-sx, -sy-1)[0] = GetAdHocAngle(-sx, -sy-1, maxval, rotation);
              _GetIndex(sx, -sy-1)[0] = GetAdHocAngle(sx, -sy-1, maxval, rotation);
            }
          }
        }
      }
    }
    return r;
  }

  Tr GetAngle(long x, long y) const
  {
    // we support negative indices here
    return _GetIndex(x, y)[0];
  }

  inline static void GetAdHocPosition(long& x, long& y, Tr angle, const Tr& radius, const Tr& maxval, const Tr& rotation)
  {
    angle -= rotation;
    angle = angle * (GetPI() * 2) / maxval;// convert to a scale of 0-2pi
    x = static_cast<long>(radius * cosf(angle));
    y = static_cast<long>(radius * sinf(angle));
  }

  // works for all 4 quadrants.  make sure that when x and y are positive, the angle is 0-.25
  inline static Tr GetAdHocAngle(long x, long y, const Tr& maxval, const Tr& rotation)
  {
    Tr a;
    if(x != 0)
    {
      a = atanf(static_cast<float>(abs(y))/abs(x)) / (2 * GetPI());
    }
    else
    {
      a = 0.25f;
    }

    // adjust for quadrants
    if(x < 0)
    {
      if(y > 0) { a = 0.50f - a; }// quadrant 2
      else { a = a + 0.50f; }// quadrant 3
    }
    else
    {
      if(y < 0) { a = 1.00f - a; }// quadrant 4
    }

    a *= maxval;
    a += rotation;
    if(a > maxval) a -= maxval;
    return a;
  }

private:
  // size of the lookup table
  unsigned long m_radius;
  Tr m_maxval;
  Tr m_rotation;

  inline Tr* _GetIndex(long x, long y) const
  {
    return m_origin + (y*m_pitch) + x;
  }

  unsigned long m_pitch;// horizontal pitch of the 2d lut
  typedef LibCC::Blob<Tr, LibCC::BlobTraits<false, 1> > BlobType;
  BlobType m_dat;
  Tr* m_origin;
};



template<typename Tr>
class RadiusLut
{
public:
  RadiusLut() :
    m_width(0),
    m_height(0)
  {
  }

  // go from 0-x and 0-y.  These are sizes, not indexes.
  bool Resize(unsigned long x, unsigned long y)
  {
    bool r = false;
    if((x <= m_width) && (y <= m_width))
    {
      r = true;
    }
    else
    {
      if((x * y) < 25000000)
      {
        m_width = x;
        m_height = y;
        if(m_dat.Realloc(x*y))
        {
          r = true;
          // generate the lut
          unsigned long sx, sy;
          unsigned long yoffset;
          for(sy = 0; sy < y; sy ++)
          {
            yoffset = sy * x;
            for(sx = 0; sx < x; sx ++)
            {
              m_dat.GetLockedBuffer()[sx + yoffset] = GetAdHocRadius(sx, sy);
            }
          }
        }
      }
    }
    return r;
  }

  Tr GetRadius(long x, long y) const
  {
    return m_dat.GetLockedBuffer()[abs(x) + (abs(y) * m_width)];
  }

  inline static Tr GetAdHocRadius(unsigned long x, unsigned long y)
  {
    return static_cast<Tr>(sqrt(static_cast<double>(x*x+y*y)));
  }

private:
  // size of the lookup table
  unsigned long m_width;
  unsigned long m_height;

  typedef LibCC::Blob<Tr, LibCC::BlobTraits<false, 1> > BlobType;
  BlobType m_dat;// we store radii in here, scaled to 0-maxval.
};


