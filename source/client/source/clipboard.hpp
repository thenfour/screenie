#ifndef SCREENIE_CLIPBOARD_HPP
#define SCREENIE_CLIPBOARD_HPP

#include "exception.hpp"

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
						std::wstring W = LibCC::ToUnicode(text);
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
								std::string A = LibCC::ToMBCS(text.c_str());
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

	LibCC::Result GetBitmap(util::shared_ptr<Gdiplus::Bitmap>& out)
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
	  LibCC::Result r;
    r.Fail();// initialize

		HBITMAP hbm = 0;
		Gdiplus::Status s = p->GetHBITMAP(0, &hbm);
		if(s != Gdiplus::Ok)
		{
			r.Fail(LibCC::Format("Error getting the HBITMAP from the Gdiplus object.").Str());
		}
		else
		{
			HBITMAP bitmapCopy = DuplicateBitmap(hbm);
			DeleteObject(hbm);
			r = SetBitmap(bitmapCopy);
		}

    return r;
	}

	LibCC::Result SetBitmap(HBITMAP bitmap)
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
					if (::SetClipboardData(CF_BITMAP, (HANDLE)bitmap) == NULL)
					{
						r.Fail(LibCC::Format("Error setting clipboard data (BITMAP). System error: %").gle(GetLastError()).Str());
					}
					else
					{
						r.Succeed();
					}
				}
        ::CloseClipboard();
      }
    }

    return r;
	}

private:
  HWND m_hOwner;

	static HBITMAP DuplicateBitmap(HBITMAP sourceBitmap)
	{
		HDC desktopDC = ::GetDC(NULL);

		BITMAP bmp = { 0 };
		if (!::GetObject(sourceBitmap, sizeof(bmp), &bmp))
			return NULL;

		HDC destDC = ::CreateCompatibleDC(desktopDC);
		HBITMAP destBitmap = ::CreateCompatibleBitmap(desktopDC, bmp.bmWidth, bmp.bmHeight);
		HGDIOBJ destOldObj = ::SelectObject(destDC, destBitmap);

		HDC sourceDC = ::CreateCompatibleDC(desktopDC);
		HGDIOBJ sourceOldObj = ::SelectObject(sourceDC, sourceBitmap);

		::BitBlt(destDC, 0, 0, bmp.bmWidth, bmp.bmHeight, sourceDC, 0, 0, SRCCOPY);

		::SelectObject(sourceDC, sourceOldObj);
		::DeleteDC(sourceDC);

		::SelectObject(destDC, destOldObj);
		::DeleteDC(destDC);

		::ReleaseDC(NULL, desktopDC);

		return destBitmap;
	}
};

#endif