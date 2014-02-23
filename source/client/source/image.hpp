// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark
//
//
//
//
//

#ifndef SCREENIE_IMAGE_HPP
#define SCREENIE_IMAGE_HPP

#include "animbitmap.h"
#include "autoGDI.hpp"

tstd::tstring GetGdiplusStatusString(Gdiplus::Status status);

tstd::tstring GetImageCodecExtension(Gdiplus::ImageCodecInfo* codecInfo, bool dotPrefix = false);

bool GetScreenshotBitmap(HBITMAP& bitmap, BOOL AltPressed, BOOL drawCursor);

bool ScaleBitmap(std::shared_ptr<Gdiplus::Bitmap>& destination, Gdiplus::Bitmap& source, float scale);
bool ResizeBitmap(std::shared_ptr<Gdiplus::Bitmap>& destination, Gdiplus::Bitmap& source, int dimensionLimit);

bool SaveImageToFile(Gdiplus::Image& image, const tstd::tstring& mimeType, const tstd::tstring& filename, ULONG quality);

bool GdiplusBitmapToAnimBitmap(Gdiplus::BitmapPtr src, AnimBitmap<32>& dest);

void DumpBitmap(Gdiplus::Bitmap& image, int x = 0, int y = 0);
void DumpBitmap(HBITMAP img, int x = 0, int y = 0);
void DumpIcon(HICON img, int x = 0, int y = 0);

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

template<int T>
void CopyImage(AnimBitmap<T>& dest, Gdiplus::Bitmap& src, int zoomFactor)
{
  dest.SetSize(src.GetWidth() * zoomFactor, src.GetHeight() * zoomFactor);

  HDC dc = ::GetDC(0);
  HDC dcc = ::CreateCompatibleDC(dc);
  ::ReleaseDC(0, dc);

  HBITMAP hScreenshot;
  src.GetHBITMAP(0, &hScreenshot);
  HBITMAP hbm = (HBITMAP)SelectObject(dcc, hScreenshot);

  dest.StretchBlitFrom(
    0, 0, src.GetWidth() * zoomFactor, src.GetHeight() * zoomFactor,
    dcc, 0, 0, src.GetWidth(), src.GetHeight(), COLORONCOLOR);

  SelectObject(dcc, hbm);
  DeleteObject(hScreenshot);
  DeleteDC(dcc);
}

// stolen from a thread on google groups
template<typename T>
inline T* GdiplusObjectFromResource(HINSTANCE hInstance, LPCTSTR szResName, LPCTSTR szResType)
{
    HRSRC hrsrc=FindResource(hInstance, szResName, szResType);
    if(!hrsrc) return 0;
    HGLOBAL hg1 = LoadResource(hInstance, hrsrc);
    DWORD sz = SizeofResource(hInstance, hrsrc);
    void* ptr1 = LockResource(hg1);
    HGLOBAL hg2 = GlobalAlloc(GMEM_FIXED, sz);
    CopyMemory(hg2, ptr1, sz);
    IStream *pStream;
    HRESULT hr=CreateStreamOnHGlobal(hg2, TRUE, &pStream);
    if(FAILED(hr)) return 0;
    T *image=T::FromStream(pStream);
    pStream->Release();
    return image;
} 

inline Gdiplus::Bitmap* BitmapFromResource(HINSTANCE hInstance, LPCTSTR szResName, LPCTSTR szResType)
{
  return GdiplusObjectFromResource<Gdiplus::Bitmap>(hInstance, szResName, szResType);
}

inline Gdiplus::Image* ImageFromResource(HINSTANCE hInstance, LPCTSTR szResName, LPCTSTR szResType)
{
  return GdiplusObjectFromResource<Gdiplus::Image>(hInstance, szResName, szResType);
} 


inline AutoGdiBitmap LoadBitmapResource(LPCTSTR szResName, LPCTSTR szResType)
{
  std::auto_ptr<Gdiplus::Bitmap> bmp(BitmapFromResource(_Module.GetResourceInstance(), szResName, szResType));
  HBITMAP hbm;
  Gdiplus::Status s = bmp->GetHBITMAP(0, &hbm);
  return hbm;
}

#endif

