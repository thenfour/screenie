/*
  LibCC Release "March 9, 2007"
  Winapi Module
  (c) 2004-2007 Carl Corcoran, carlco@gmail.com
  Documentation: http://wiki.winprog.org/wiki/LibCC

	== License:

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

#pragma once

#ifdef WIN32

#include <string>
#include "StringUtil.h"
#include "blob.h"


#include <windows.h>// for windows types
#include <shlwapi.h>// for Path* functions


// Set up inline option
#ifdef _MSC_VER
# if (LIBCC_OPTION_INLINE == 1)// set this option
#   define LIBCC_INLINE __declspec(noinline)
# else
#   define LIBCC_INLINE inline
# endif
#else
# define LIBCC_INLINE inline
#endif


namespace LibCC
{
  // Win32 Wrappers Declaration -----------------------------------------------------------------------------------
  template<typename Traits, typename Alloc>
  LIBCC_INLINE void FormatMessageGLE(std::basic_string<wchar_t, Traits, Alloc>& out, int code);
  template<typename Char, typename Traits, typename Alloc>
  LIBCC_INLINE void FormatMessageGLE(std::basic_string<Char, Traits, Alloc>& out, int code);

  template<typename Traits, typename Alloc>
  LIBCC_INLINE bool LoadStringX(HINSTANCE hInstance, UINT stringID, std::basic_string<wchar_t, Traits, Alloc>& out);
  template<typename Char, typename Traits, typename Alloc>
  LIBCC_INLINE bool LoadStringX(HINSTANCE hInstance, UINT stringID, std::basic_string<Char, Traits, Alloc>& out);

  // RegCreateKeyEx
  template<typename Char>
  inline LONG RegCreateKeyExX(HKEY hKey, const Char* szSubKey, DWORD dwOptions, REGSAM Sam, PHKEY pResult, DWORD* pdwDisposition)
  {
    std::wstring buf;
		ConvertString(szSubKey, buf);
    return RegCreateKeyExW(hKey, buf.c_str(), 0, 0, dwOptions, Sam, 0, pResult, pdwDisposition);
  }

  inline LONG RegCreateKeyExX(HKEY hKey, const wchar_t* szSubKey, DWORD dwOptions, REGSAM Sam, PHKEY pResult, DWORD* pdwDisposition)
  {
    return RegCreateKeyExW(hKey, szSubKey, 0, 0, dwOptions, Sam, 0, pResult, pdwDisposition);
  }

  // RegOpenKeyEx
  template<typename Char>
  inline LONG RegOpenKeyExX(HKEY hKey, const Char* szSubKey, DWORD dwOptions, REGSAM Sam, PHKEY pResult)
  {
    std::wstring buf = ToUnicode(szSubKey);
    return RegOpenKeyExW(hKey, buf.c_str(), dwOptions, Sam, pResult);
  }

  inline LONG RegOpenKeyExX(HKEY hKey, const wchar_t* szSubKey, DWORD dwOptions, REGSAM Sam, PHKEY pResult)
  {
    LONG l = RegOpenKeyExW(hKey, szSubKey, dwOptions, Sam, pResult);
    return l;
  }

  // RegDeleteKeyEx
  inline LONG RegDeleteValueX(HKEY hKey, PCWSTR lpValueName)
  {
    return RegDeleteValueW(hKey, lpValueName);
  }
  template<typename Char>
  inline LONG RegDeleteValueX(HKEY hKey, const Char* lpValueName)
  {
    std::wstring buf = ToUnicode(lpValueName);
    return RegDeleteValueW(hKey, buf.c_str());
  }

  // RegSetValueEx
  template<typename Char>
  inline LONG RegSetValueExStringX( HKEY hKey, const Char* lpValueName, DWORD Reserved, DWORD dwType, const Char* strX)
  {
    std::wstring valueName;
		std::wstring strW = ToUnicode(strX);
    if(lpValueName)
    {
      ConvertString(lpValueName, valueName);
    }
    return RegSetValueExW(hKey, lpValueName ? valueName.c_str() : 0, Reserved, REG_SZ, (const BYTE*)(strW.c_str()), (DWORD)(strW.size() + 1) * sizeof(wchar_t));
  }
  inline LONG RegSetValueExStringX( HKEY hKey, const wchar_t* lpValueName, DWORD Reserved, DWORD, const wchar_t* str)
  {
    return RegSetValueExW(hKey, lpValueName, Reserved, REG_SZ, (const BYTE*)str, (DWORD)(wcslen(str) + 1) * sizeof(wchar_t));
  }

  inline LONG RegSetValueExX(HKEY hKey, const wchar_t* lpValueName, DWORD dwType, const BYTE* lpData, DWORD cbData)
  {
    return RegSetValueExW(hKey, lpValueName, 0, dwType, lpData, cbData);
  }
  template<typename Char>
  inline LONG RegSetValueExX( HKEY hKey, const Char* lpValueName, DWORD dwType, const BYTE* lpData, DWORD cbData)
  {
    std::wstring valueName;
    if(lpValueName)
    {
			valueName = ToUnicode(lpValueName);
    }
    return RegSetValueExX(hKey, lpValueName ? valueName.c_str() : 0, dwType, lpData, cbData);
  }

  // RegQueryValueEx
  inline LONG RegQueryValueExStringX(HKEY hKey, const wchar_t* lpValueName, DWORD, DWORD, std::wstring& str)
  {
    // get the length of the thing.
    DWORD size;
    DWORD type;
    LONG r;
    if(ERROR_SUCCESS == (r = RegQueryValueExW(hKey, lpValueName, 0, &type, 0, &size)))
    {
      if(type == REG_SZ)
      {
        Blob<BYTE> buf;
        buf.Alloc(size + sizeof(wchar_t));
        size = (DWORD)buf.Size();
        if(ERROR_SUCCESS == (r = RegQueryValueExW(hKey, lpValueName, 0, 0, buf.GetBuffer(), &size)))
        {
          str = (const wchar_t*)buf.GetBuffer();
        }
      }
    }
    return r;
  }
  template<typename Char>
  inline LONG RegQueryValueExStringX(HKEY hKey, const Char* lpValueName, DWORD Reserved, std::basic_string<Char>& strX)
  {
    std::wstring valueName;
    std::wstring strW;
    if(lpValueName)
    {
      ConvertString(lpValueName, valueName);
    }
    LONG r;
    if(ERROR_SUCCESS == (r = RegQueryValueExStringX(hKey, lpValueName ? valueName.c_str() : 0, Reserved, REG_SZ, strW)))
    {
      ConvertString(strW, strX);
    }
    return r;
  }

  inline LONG RegQueryValueExX(HKEY hKey, const wchar_t* valueName, DWORD* type, BYTE* data, DWORD* cbData)
  {
    return RegQueryValueExW(hKey, valueName, 0, type, data, cbData);
  }
  template<typename Char>
  inline LONG RegQueryValueExX(HKEY hKey, const Char* szValueName, DWORD* type, BYTE* data, DWORD* cbData)
  {
    std::wstring valueName;
    if(szValueName)
    {
      std::wstring strW;
      ConvertString(szValueName, valueName);
    }
    return RegQueryValueExX(hKey, szValueName ? valueName.c_str() : 0, type, data, cbData);
  }

  // RegDeleteKey
  inline LONG RegDeleteKeyX(HKEY hKey, const wchar_t* subKey)
  {
    return RegDeleteKeyW(hKey, subKey);
  }
  template<typename Char>
  inline LONG RegDeleteKeyX(HKEY hKey, const Char* subKey)
  {
    std::wstring strW;
    StringCopy(strW, subKey);
    return RegDeleteKeyW(hKey, strW);
  }

  // RegEnumKeyEx
  // returns ERROR_NO_MORE_ITEMS or ERROR_SUCCESS
  // if maxNameSize == 0, then we will compute it for you.  Use it between calls on the same key for performance
  inline LONG RegEnumKeyExX(HKEY hKey, DWORD dwIndex, std::wstring& outName, DWORD& maxNameSize)
  {
    FILETIME temp;
    LONG ret;
    Blob<wchar_t> buf;
    DWORD size;

    outName.clear();

    // get maximum subkey name length.
    if(!maxNameSize)
    {
      if(ERROR_SUCCESS != (ret = RegQueryInfoKeyW(hKey, 0, 0, 0, 0, &maxNameSize, 0, 0, 0, 0, 0, 0)))
      {
        return ret;
      }
      maxNameSize += 2;// for safety
    }

    // allocate the memory
    if(!buf.Alloc(maxNameSize))
    {
      return ERROR_OUTOFMEMORY;
    }

    // make the call
    size = static_cast<DWORD>(buf.Size());
    ret = RegEnumKeyExW(hKey, dwIndex, buf.GetBuffer(), &size, 0, 0, 0, &temp);
    if(ret == ERROR_SUCCESS)
    {
      outName = buf.GetBuffer();
    }

    return ret;
  }
  inline LONG RegEnumKeyExX(HKEY hKey, DWORD dwIndex, std::wstring& outName)
  {
    DWORD maxNameSize = 0;
    return RegEnumKeyExX(hKey, dwIndex, outName, maxNameSize);
  }
  template<typename Char>
  inline LONG RegEnumKeyExX(HKEY hKey, DWORD dwIndex, std::basic_string<Char>& outName, DWORD& maxNameSize)
  {
    std::wstring outNameW;
    LONG ret = RegEnumKeyExX(hKey, dwIndex, outNameW, maxNameSize);
		ConvertString(outNameW, outName);
    return ret;
  }
  template<typename Char>
  inline LONG RegEnumKeyExX(HKEY hKey, DWORD dwIndex, std::basic_string<Char>& outName)
  {
    std::wstring outNameW;
    DWORD maxNameSize = 0;
    LONG ret = RegEnumKeyExX(hKey, dwIndex, outNameW, maxNameSize);
		ConvertString(outNameW, outName);
    return ret;
  }

  inline bool IsValidHandle(HANDLE a)
  {
    return (a != 0) && (a != INVALID_HANDLE_VALUE);
  }

  inline bool IsBadHandle(HANDLE a)
  {
    return !IsValidHandle(a);
  }
  #define E_TERRORISM_NOT_RULED_OUT E_FAIL
  template<typename Char>
  inline bool PathIsAbsolute(const std::basic_string<Char>& path)
  {
      bool r = false;
      if(path.length() >= 3)
      {
          std::basic_string<Char> sSearch = path.substr(1, 2);
          if(XStringEquals(sSearch, ":/") || XStringEquals(sSearch, ":\\"))
          {
              r = true;
          }
      }
      return r;
  }

  template<typename Char>
  std::basic_string<Char> PathRemoveFilename(const std::basic_string<Char>& path)
  {
      std::basic_string<Char>::size_type nLastSlash = 0;
      nLastSlash = XStringFindLastOf(path, "\\/");
      if(nLastSlash == std::string::npos) return path;

      return path.substr(0, nLastSlash);
  }

  template<typename Char>
  inline bool IsPathSeparator(Char c)
  {
    switch(c)
    {
    case '\\':
    case '/':
      return true;
    }
    return false;
  }

  template<typename Char>
  std::basic_string<Char> PathJoin(const std::basic_string<Char>& dir, const std::basic_string<Char>& file)
  {
    if(dir.empty()) return file;
    Char c = *(dir.rbegin());
    switch(c)
    {
    case '\\':
    case '/':
      return dir + file;
    }
    std::basic_string<Char> ret(dir);
    ret.push_back('\\');
    ret.append(file);
    return ret;
  }

  template<typename Char, typename Traits, typename Alloc>
  bool GetCurrentDirectoryX(std::basic_string<Char, Traits, Alloc>& out)
  {
    Blob<Char> buf;
    //TCHAR* tsz = 0;
    DWORD dwRet = 0;
    bool r = false;

    dwRet = GetCurrentDirectoryW(static_cast<DWORD>(buf.Size()), buf.GetBuffer());
    if(dwRet)
    {
      if(dwRet > static_cast<DWORD>(buf.Size()))
      {
        // It needs more space.
        if(buf.Alloc(dwRet+2))
        {
          if(::GetCurrentDirectory(static_cast<DWORD>(buf.Size()), buf.GetBuffer()))
          {
            out = buf.GetBuffer();
            r = true;
          }
        }
      }
      else
      {
        out = buf.GetBuffer();
        r = true;
      }
    }

    return r;
  }

  // GetTempPath
  template<typename Char, typename Traits, typename Alloc>
  inline bool GetTempPathX(std::basic_string<Char, Traits, Alloc>& sOut)
  {
    bool r = false;
    DWORD size;
    wchar_t temp[2];

    size = GetTempPathW(1, temp);
    if(size)
    {
      BlobTypes<wchar_t>::PathBlob buf;
      buf.Alloc(size + 1);
      if(GetTempPathW(buf.Size(), buf.GetWritableBuffer()))
      {
        StringCopy(sOut, buf.GetBuffer());
        r = true;
      }
    }
    return r;
  }

  // LoadLibrary
  template<typename Char>
  inline HMODULE LoadLibraryX(const Char* buffer)
  {
    Blob<wchar_t, false, BlobTraits<true, MAX_PATH> > t;
    t.Alloc(StringLength(buffer) + 1);
    t.GetWritableBuffer()[0] = 0;
    StringCopy(t.GetWritableBuffer(), buffer);
    return LoadLibraryW(t.GetBuffer());
  }

  inline HMODULE LoadLibraryX(const wchar_t* buffer)
  {
    return LoadLibraryW(buffer);
  }

  // LoadString
  template<typename Char>
  inline int LoadStringX(HINSTANCE hInstance, UINT id, Char* buffer, int buffermax)
  {
    Blob<wchar_t, false, BlobTraits<true, 1024> > t;
    t.Alloc(buffermax + 1);
    t.GetWritableBuffer()[0] = 0;
    int r = LoadStringW(hInstance, id, t.GetWritableBuffer(), buffermax);
    StringCopy(buffer, t.GetBuffer());
    return r;
  }

  inline int LoadStringX(HINSTANCE hInstance, UINT id, wchar_t* buffer, int buffermax)
  {
    return LoadStringW(hInstance, id, buffer, buffermax);
  }

  // PathAppend
  template<typename Char, typename Traits, typename Alloc>
  inline void PathAppendX(IN OUT std::basic_string<Char, Traits, Alloc>& sPath, const std::basic_string<Char, Traits, Alloc>& sFilename)
  {
    // add trailing backslash if needed
    if(sPath.size() && sPath[sPath.size() - 1] != '\\')
    {
      sPath.push_back('\\');
    }

    sPath.append(sFilename);
  }

  template<typename Char>
  inline bool PathFileExistsX(const Char* path)
  {
    BlobTypes<Char>::PathBlob t;
    StringCopy(t, path);
    bool r = (TRUE == PathFileExistsW(t.GetBuffer()));
    return r;
  }

  inline int PathFileExistsX(const wchar_t* path)
  {
    return TRUE == PathFileExistsW(path);
  }

  // SHGetSpecialFolderPath
  template<typename Char, typename Traits, typename Alloc>
  inline bool SHGetSpecialFolderPathX(HWND hOwner, std::basic_string<Char, Traits, Alloc>& sOut, int nFolder, bool create)
  {
    // TODO support pathnames longer than 300 chars
    bool r = false;
    BlobTypes<wchar_t>::PathBlob buf;
    if(TRUE == SHGetSpecialFolderPathW(hOwner, buf.GetWritableBuffer(), nFolder, create))
    {
      StringCopy(sOut, buf.GetBuffer());
      r = true;
    }
    return r;
  }
}











//////////////////////////////////////////////////////////////////////////////////////////////////////////
/// IMPLEMENTATION ////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////



namespace LibCC
{
  // Win32 wrappers Implementation  -----------------------------------------------------------------------------------
  template<typename Traits, typename Alloc>
  LIBCC_INLINE void FormatMessageGLE(std::basic_string<wchar_t, Traits, Alloc>& out, int code)
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
  LIBCC_INLINE void FormatMessageGLE(std::basic_string<Char, Traits, Alloc>& out, int code)
  {
    std::wstring s;
    FormatMessageGLE(s, code);
    StringCopy(out, s);
    return;
  }

  template<typename Traits, typename Alloc>
  LIBCC_INLINE bool LoadStringX(HINSTANCE hInstance, UINT stringID, std::basic_string<wchar_t, Traits, Alloc>& out)
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

}


#endif// WIN32



