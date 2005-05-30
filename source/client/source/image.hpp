//
//
//
//
//

#ifndef SCREENIE_IMAGE_HPP
#define SCREENIE_IMAGE_HPP

#include "animbitmap.h"

tstd::tstring GetGdiplusStatusString(Gdiplus::Status status);

tstd::tstring GetImageCodecExtension(Gdiplus::ImageCodecInfo* codecInfo, bool dotPrefix = false);

bool GetScreenshotBitmap(HBITMAP& bitmap, const RECT& rectangle, BOOL drawCursor);

bool ScaleBitmap(util::shared_ptr<Gdiplus::Bitmap>& destination, Gdiplus::Bitmap& source, float scale);
bool ResizeBitmap(util::shared_ptr<Gdiplus::Bitmap>& destination, Gdiplus::Bitmap& source, int dimensionLimit);

bool SaveImageToFile(Gdiplus::Image& image, const tstd::tstring& mimeType, const tstd::tstring& filename);

void DumpBitmap(Gdiplus::Bitmap& image, int x = 0, int y = 0);

template<int T>
void DumpBitmap(AnimBitmap<T>& img, int x = 0, int y = 0)
{
  // draw that damn bitmap to the screen.
  HDC dc = ::GetDC(0);
  img.StretchBlit(dc, x, y, img.GetWidth() / 2, img.GetHeight() / 2);
  ::ReleaseDC(0,dc);
}

template<int T>
void CopyImage(AnimBitmap<T>& dest, Gdiplus::Bitmap& src)
{
  dest.SetSize(src.GetWidth(), src.GetHeight());

  HDC dc = ::GetDC(0);
  HDC dcc = ::CreateCompatibleDC(dc);
  ::ReleaseDC(0, dc);

  HBITMAP hScreenshot;
  src.GetHBITMAP(0, &hScreenshot);
  HBITMAP hbm = (HBITMAP)SelectObject(dcc, hScreenshot);
  dest.BlitFrom(dcc, 0, 0, src.GetWidth(), src.GetHeight());
  SelectObject(dcc, hbm);
  DeleteObject(hScreenshot);
  DeleteDC(dcc);
}

#endif