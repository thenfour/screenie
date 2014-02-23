// http://screenie.net
// Copyright (c) 2003-2009 Carl Corcoran & Roger Clark

#ifndef SCREENIE_CLIPBOARD_HPP
#define SCREENIE_CLIPBOARD_HPP

#include "exception.hpp"
#include "codec.hpp"

/*
  RetryEngine re(3, 500);
  for(re.Begin(); re.End(status); re.Next())
  {
    status = SomeFunc();
    if(status)
    {
      status = SomeOtherFunc();
    }
  }
  ...
*/
class RetryEngine
{
public:
  RetryEngine(int n, DWORD wait) :
    m_retriesLeft(n),
    m_retriesTotal(n),
    m_wait(wait)
  {
  }
  /*
    condition is AND'd into the test. if condition is false (like an error), then we will continue to retry.
    otherwise if it is TRUE, then we end the loop.
  */
  bool End(bool condition) { return (0 == m_retriesLeft) || condition; }
  void Begin() { m_retriesLeft = m_retriesTotal; }
  void Next(bool condition)// prevent waiting if End() is true.
  {
    if(End(condition)) return;
    Sleep(m_wait);
    m_retriesLeft --;
  }
private:
  int m_retriesTotal;
  int m_retriesLeft;
  DWORD m_wait;
};

class Clipboard
{
public:
  Clipboard(HWND owner) :
    m_hOwner(owner)
	{
	}

	~Clipboard()
	{
	}

  LibCC::Result SetText(const tstd::tstring& text)
	{
    LibCC::Result r;
    r.Fail();// initialize
    RetryEngine re(4, 500);

    for(re.Begin(); !re.End(r.Succeeded()); re.Next(r.Succeeded()))
    {
		  if (!::OpenClipboard(m_hOwner))
      {
        r.Fail(LibCC::Format("Error getting access to the clipboard. System error: %").gle(GetLastError()).Str());
      }
      else
      {
        if(!EmptyClipboard())
        {
          r.Fail(LibCC::Format("Error getting access to the clipboard. System error: %").gle(GetLastError()).Str());
        }
        else
        {
          size_t nSize;
          HANDLE hMem;
          PVOID hMemLoc;

          // unicode first
          nSize = (text.length() + 1) * sizeof(WCHAR);
          hMem = GlobalAlloc(GMEM_MOVEABLE, nSize);
		      if (hMem == NULL)
          {
            r.Fail(LibCC::Format("Error allocating clipboard memory (UNICODE). System error: %").gle(GetLastError()).Str());
          }
          else
          {
            hMemLoc = GlobalLock(hMem);
						std::wstring W = LibCC::ToUTF16(text);
						wcscpy((WCHAR*)hMemLoc, W.c_str());
            GlobalUnlock(hMem);
            if(NULL == SetClipboardData(CF_UNICODETEXT, hMem))
            {
              r.Fail(LibCC::Format("Error setting clipboard data (UNICODE). System error: %").gle(GetLastError()).Str());
            }
            else
            {
              nSize = (text.length() + 1) * sizeof(char);
              hMem = GlobalAlloc(GMEM_MOVEABLE, nSize);
		          if (hMem == NULL)
              {
                r.Fail(LibCC::Format("Error allocating clipboard memory (ANSI). System error: %").gle(GetLastError()).Str());
              }
              else
              {
                hMemLoc = GlobalLock(hMem);
								std::string A = LibCC::ToANSI(text.c_str());
								strcpy((char*)hMemLoc, A.c_str());
                GlobalUnlock(hMem);
                if(NULL == SetClipboardData(CF_TEXT, hMem))
                {
                  r.Fail(LibCC::Format("Error setting clipboard data (ANSI). System error: %").gle(GetLastError()).Str());
                }
                else
                {
                  r.Succeed();
                }
              }
            }
          }
        }
        ::CloseClipboard();
      }
    }

    return r;
	}

	static bool ContainsBitmap()
	{
		return TRUE == IsClipboardFormatAvailable(CF_BITMAP);
	}

	LibCC::Result GetBitmap(std::shared_ptr<Gdiplus::Bitmap>& out)
	{
    LibCC::Result r;
	  if (!::OpenClipboard(m_hOwner))
    {
      r.Fail(LibCC::Format("Error getting access to the clipboard. System error: %").gle(GetLastError()).Str());
    }
    else
    {
			HBITMAP h = (HBITMAP)GetClipboardData(CF_BITMAP);
			if(!h)
			{
				r.Fail(LibCC::Format("Error retreiving clipboard data. Maybe there is no bitmap in the clipboard. System error: %").gle(GetLastError()).Str());
			}
			else
			{
				out.reset(Gdiplus::Bitmap::FromHBITMAP(h, 0));
				r.Succeed();
			}
      ::CloseClipboard();
    }
		return r;
	}

	LibCC::Result SetBitmap(Gdiplus::Bitmap* p)
	{
		LibCC::Result r = LibCC::Result::Failure();

		// below code mostly stolen from:
		// http://groups.google.be/group/borland.public.cppbuilder.graphics/browse_thread/thread/80375248929e810a/17daa3b660488594?hl=en&lnk=st&q=GetDIBits+cf_dib+setclipboarddata+BITMAPINFO#17daa3b660488594
		HBITMAP hbm = 0;
		if(Gdiplus::Ok != p->GetHBITMAP(0, &hbm))
		{
			r.Fail(L"Error getting access to the original bitmap.");
		}
		else
		{
			HANDLE HMem;
			BITMAP bm = {0};
			if(0 == GetObject(hbm, sizeof(bm), &bm))
			{
				r.Fail(LibCC::Format(L"Error getting info about the image. ").gle().Str());
			}
			else
			{
				LPBITMAPINFO lpBitmapInfo;
				lpBitmapInfo  = (BITMAPINFO*)GlobalAlloc(GMEM_FIXED, sizeof(BITMAPINFO));
				ZeroMemory(lpBitmapInfo, sizeof(BITMAPINFO));
				lpBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				lpBitmapInfo->bmiHeader.biWidth = bm.bmWidth;
				lpBitmapInfo->bmiHeader.biHeight = bm.bmHeight;
				lpBitmapInfo->bmiHeader.biPlanes = 1;
				lpBitmapInfo->bmiHeader.biBitCount = bm.bmBitsPixel;
				lpBitmapInfo->bmiHeader.biCompression = BI_RGB;

				HDC ScreenDC = GetDC(NULL);

				GetDIBits(ScreenDC, hbm, 0, lpBitmapInfo->bmiHeader.biHeight, NULL, lpBitmapInfo, DIB_RGB_COLORS);
				if (lpBitmapInfo->bmiHeader.biSizeImage == 0)
				{
					lpBitmapInfo->bmiHeader.biSizeImage = ((((bm.bmWidth * bm.bmBitsPixel) + 31) & ~31) / 8) * bm.bmHeight;
				}

				SIZE_T s = lpBitmapInfo->bmiHeader.biSizeImage + lpBitmapInfo->bmiHeader.biSize;
				HANDLE HDib = GlobalReAlloc(lpBitmapInfo, s, GMEM_MOVEABLE | GMEM_DDESHARE);

				if(!HDib)
				{
					r.Fail(LibCC::Format(L"Error allocating image memory (% bytes). ").i(s).gle().Str());
				}
				else
				{
					HMem = GlobalLock(HDib);

					LPBITMAPINFO lpbih = (LPBITMAPINFO)HMem;

					int num_scanlines = GetDIBits(ScreenDC, hbm, 0, lpbih->bmiHeader.biHeight, (LPBYTE)lpbih + lpbih->bmiHeader.biSize, lpbih, DIB_RGB_COLORS);
					GlobalUnlock(HDib);

					if (num_scanlines == 0)
					{
						r.Fail(LibCC::Format(L"Error getting bitmap image data. ").gle().Str());
					}
					else
					{
						OpenClipboard(m_hOwner);
						EmptyClipboard();
						SetClipboardData(CF_DIB, HDib);
						CloseClipboard();
						r.Succeed();
					}
					ReleaseDC(NULL, ScreenDC);
				}
			}

			DeleteObject(hbm);
		}

		return r;
	}

	LibCC::Result SetBitmap(HBITMAP bitmap)
	{
		Gdiplus::Bitmap bmTemp(bitmap, 0);
		return SetBitmap(&bmTemp);
	}

private:
  HWND m_hOwner;
};

#endif