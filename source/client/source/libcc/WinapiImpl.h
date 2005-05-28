/*
  Last updated Jan 9, 2005

  (c) 2004-2005 Carl Corcoran, carl@ript.net
  http://carl.ript.net/stringformat/
  http://carl.ript.net/wp
  http://mantis.winprog.org/
  http://svn.winprog.org/personal/carlc/stringformat

  All software on this site is provided 'as-is', without any express or
  implied warranty, by its respective authors and owners. In no event will
  the authors be held liable for any damages arising from the use of this
  software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
  claim that you wrote the original software. If you use this software in
  a product, an acknowledgment in the product documentation would be
  appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not
  be misrepresented as being the original software.

  3. This notice may not be removed or altered from any source distribution.
*/

// This is an inline h file for implementation.  Do not #include it.


namespace LibCC
{
#ifdef CCSTR_WIN32
  // Win32 wrappers Implementation  -----------------------------------------------------------------------------------
  template<typename Traits, typename Alloc>
  CCSTR_INLINE void FormatMessageGLE(std::basic_string<wchar_t, Traits, Alloc>& out, int code)
  {
    wchar_t* lpMsgBuf(0);
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
      0, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL);
    if(lpMsgBuf)
    {
      out = lpMsgBuf;
      LocalFree(lpMsgBuf);
    }
    else
    {
      out = L"Unknown error: ";
      wchar_t temp[50];
      _itow(code, temp, 50);
      out.append(temp);
    }
  }

  template<typename Char, typename Traits, typename Alloc>
  CCSTR_INLINE void FormatMessageGLE(std::basic_string<Char, Traits, Alloc>& out, int code)
  {
    std::wstring s;
    FormatMessageGLE(s, code);
    StringCopy(out, s);
    return;
  }

  template<typename Traits, typename Alloc>
  CCSTR_INLINE bool LoadStringX(HINSTANCE hInstance, UINT stringID, std::basic_string<wchar_t, Traits, Alloc>& out)
  {
    static const int StaticBufferSize = 1024;
    static const int MaximumAllocSize = 5242880;// don't attempt loading strings larger than 10 megs
    bool r = false;
    wchar_t temp[StaticBufferSize];// start with fast stack buffer
    // LoadString returns the # of chars copied, not including the null terminator
    if(LoadStringW(hInstance, stringID, temp, StaticBufferSize) < (StaticBufferSize - 1))
    {
      out = temp;
      r = true;
    }
    else
    {
      // we loaded up the maximum size; the string was probably truncated.
      int size = StaticBufferSize * 2;// this # is in chars, not bytes
      while(1)
      {
        // allocate a buffer.
        if(size > MaximumAllocSize)
        {
          // failed... too large of a string.
          break;
        }
        wchar_t* buf = static_cast<wchar_t*>(HeapAlloc(GetProcessHeap(), 0, size * sizeof(wchar_t)));
        if(LoadStringW(hInstance, stringID, buf, size) < (size - 1))
        {
          // got what we wanted.
          out = buf;
          HeapFree(GetProcessHeap(), 0, buf);
          r = true;
          break;
        }
        HeapFree(GetProcessHeap(), 0, buf);
        size <<= 1;// double the amount to allocate
      }
    }
    return r;
  }

  template<typename Char, typename Traits, typename Alloc>
  bool LoadStringX(HINSTANCE hInstance, UINT stringID, std::basic_string<Char, Traits, Alloc>& out)
  {
    bool r = false;
    std::wstring ws;
    if(LoadStringX(hInstance, stringID, ws))
    {
      r = true;
      StringCopy(out, ws);
    }
    return r;
  }

#endif // _WIN32
}

