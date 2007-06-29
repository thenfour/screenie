
#ifndef SCREENIE_PROGRESSIMAGES_HPP
#define SCREENIE_ACTIVITYLISTITEM_HPP

#include "animbitmap.h"
#include "polarlut.h"

class ProgressImages
{
public:
  void InitializeProgressImages(CImageList& img, RgbPixel32 background, RgbPixel32 filled, RgbPixel32 unfilled);
  int GetImageFromProgress(int pos, int total);
private:
  /*
    this will hold linear from 0-perimiter.
    assume that the image list indices will not change.
  */
  std::vector<int> m_images;
  AngleLut<float> m_angles;

  void DrawHLine(long x1, long x2, long y);
  void DrawAlphaPixel(long cx, long cy, long x, long y, long f, long fmax);
  RgbPixel32 PositionToColor(long x, long y);

  // stuff that we persist between drawing calls
  AnimBitmap<32>* m_bmp;
  int m_i;

  float m_pieBlurringSize;
  int m_perimeter;
  int m_diameter;
  int m_radius;
  RgbPixel32 m_background;
  RgbPixel32 m_unfilled;
  RgbPixel32 m_filled;
};


#endif

