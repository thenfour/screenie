

#include "stdafx.hpp"
#include "ProgressImages.hpp"
#include "geom.h"


void ProgressImages::DrawHLine(long x1, long x2, long y)
{
  // draw horizontal line.
  long xleft = min(x1, x2);
  long xright = max(x1, x2) + 1;
  while(xleft != xright)
  {
    m_bmp->SetPixel(xleft, y, PositionToColor(xleft, y));
    xleft ++;
  }
}

void ProgressImages::DrawAlphaPixel(long cx, long cy, long x, long y, long f, long fmax)
{
  m_bmp->SetPixel(cx+x, cy+y, MixColorsInt(f, fmax, PositionToColor(cx+x, cy+y), m_background));
  m_bmp->SetPixel(cx+x, cy-y-1, MixColorsInt(f, fmax, PositionToColor(cx+x, cy-y-1), m_background));
  m_bmp->SetPixel(cx-x-1, cy+y, MixColorsInt(f, fmax, PositionToColor(cx-x-1, cy+y), m_background));
  m_bmp->SetPixel(cx-x-1, cy-y-1, MixColorsInt(f, fmax, PositionToColor(cx-x-1, cy-y-1), m_background));
}

// x and y are relative to the 
RgbPixel32 ProgressImages::PositionToColor(long x, long y)
{
  if(m_i == m_perimeter) return m_filled;// 100% one is all filled.
  float a = m_angles.GetAngle(x - m_radius, y - m_radius);// GetAngle() needs coords relative to the center of the circle

  // rotate it 90 degrees counter-clockwise.
  a += static_cast<float>(m_perimeter) / 4;
  if(a < 0) a += m_perimeter;
  if(a > m_perimeter) a -= m_perimeter;

  /* to fake anti-aliasing inside the pie, just calculate using a narrow gradient.
   --filled---k====a====l----unfilled------
              |---------| = blur size.
    the gradient is from k to l.  i may be anywhere in this diagram.
  */
  // get the distance from i to the center of the blurring window.
  float aa = a + (m_pieBlurringSize / 2) - static_cast<float>(m_i);
  if(aa <= 0) return m_filled;
  if(aa > m_pieBlurringSize) return m_unfilled;
  // multiply by 100 because we are casting to integer
  return MixColorsInt(static_cast<long>(aa * 100.0f), static_cast<long>(m_pieBlurringSize * 100.0f), m_unfilled, m_filled);
}

void ProgressImages::InitializeProgressImages(CImageList& img, RgbPixel32 background, RgbPixel32 filled, RgbPixel32 unfilled)
{
  m_background = background;
  m_unfilled = unfilled;
  m_filled = filled;
  m_diameter = 14;
  m_radius = 7;
  m_pieBlurringSize = 2;
  m_perimeter = static_cast<int>(GetPI() * m_diameter);
  m_background = background;
  m_images.clear();
  m_images.reserve(m_perimeter+1);

  // add 1 for complete coverage, to be on the safe side.
  m_angles.Resize(m_radius + 1, static_cast<float>(m_perimeter), 0);// the values returned from m_angles will be in the range 0-perimeter, or in other words, 0-m_images.size().

  for(m_i = 0; m_i < m_perimeter; m_i ++)
  {
    m_bmp = new AnimBitmap<32>();
    m_bmp->SetSize(16, 16);
    m_bmp->Fill(m_background);
	FilledCircleAAG(8, 8, m_radius, this, &ProgressImages::DrawHLine, this, &ProgressImages::DrawAlphaPixel);
    // add it to the imagelist.
    HBITMAP hbm = m_bmp->DetachHandle();
    m_images.push_back(img.Add(hbm));
    DeleteObject(hbm);
    delete m_bmp;
    m_bmp = 0;
  }
}

int ProgressImages::GetImageFromProgress(int pos, int total)
{
  // pos / total = index / m_images.size();
  int index = (int)(0.05f + (float)pos * (float)m_images.size() / (float)total);
  // bounds checking.
  if(index < 0) index = 0;
  if(index >= (int)m_images.size()) index = m_images.size() - 1;
  return m_images[index];
}
