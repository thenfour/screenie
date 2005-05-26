//
//
//
//
//

#ifndef SCREENIE_IMAGE_HPP
#define SCREENIE_IMAGE_HPP

tstd::tstring GetGdiplusStatusString(Gdiplus::Status status);

tstd::tstring GetImageCodecExtension(Gdiplus::ImageCodecInfo* codecInfo, bool dotPrefix = false);

bool GetScreenshotBitmap(HBITMAP& bitmap, const RECT& rectangle, BOOL drawCursor);

bool ScaleBitmap(util::shared_ptr<Gdiplus::Bitmap>& destination, Gdiplus::Bitmap& source, float scale);
bool ResizeBitmap(util::shared_ptr<Gdiplus::Bitmap>& destination, Gdiplus::Bitmap& source, int dimensionLimit);

bool SaveImageToFile(Gdiplus::Image& image, const tstd::tstring& mimeType, const tstd::tstring& filename);

#endif