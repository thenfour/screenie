/*
  Last updated May 26, 2005

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

#pragma once

#include "libccoptions.h"

#include <string>
#include "StringBase.h"
#include "blob.h"

#ifdef CCSTR_WIN32
# include <windows.h>// for windows types
# include <shlwapi.h>// for Path* functions
#endif

namespace LibCC
{
  // Win32 Wrappers Declaration -----------------------------------------------------------------------------------
#ifdef CCSTR_WIN32
  template<typename Traits, typename Alloc>
  CCSTR_INLINE void FormatMessageGLE(std::basic_string<wchar_t, Traits, Alloc>& out, int code);
  template<typename Char, typename Traits, typename Alloc>
  CCSTR_INLINE void FormatMessageGLE(std::basic_string<Char, Traits, Alloc>& out, int code);

  template<typename Traits, typename Alloc>
  CCSTR_INLINE bool LoadStringX(HINSTANCE hInstance, UINT stringID, std::basic_string<wchar_t, Traits, Alloc>& out);
  template<typename Char, typename Traits, typename Alloc>
  CCSTR_INLINE bool LoadStringX(HINSTANCE hInstance, UINT stringID, std::basic_string<Char, Traits, Alloc>& out);

  // RegCreateKeyEx
  template<typename Char>
  inline LONG RegCreateKeyExX(HKEY hKey, const Char* szSubKey, DWORD dwOptions, REGSAM Sam, PHKEY pResult, DWORD* pdwDisposition)
  {
    //BlobTypes<wchar_t>::PathBlob buf;
    std::wstring buf;
    StringCopy(buf, szSubKey);
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
    //BlobTypes<wchar_t>::PathBlob buf;
    std::wstring buf;
    StringCopy(buf, szSubKey);
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
    std::wstring buf;
    StringCopy(buf, lpValueName);
    return RegDeleteValueW(hKey, buf.c_str());
  }

  // RegSetValueEx
  template<typename Char>
  inline LONG RegSetValueExStringX( HKEY hKey, const Char* lpValueName, DWORD Reserved, DWORD dwType, const Char* strX)
  {
    std::wstring valueName;
    std::wstring strW;
    if(lpValueName)
    {
      StringCopy(valueName, lpValueName);
      StringCopy(strW, strX);
    }
    return RegSetValueExW(hKey, lpValueName ? valueName.c_str() : 0, Reserved, REG_SZ, (const BYTE*)(strW.c_str()), (DWORD)(strW.size() + 1) * sizeof(wchar_t));
  }
  inline LONG RegSetValueExStringX( HKEY hKey, const wchar_t* lpValueName, DWORD Reserved, DWORD dwType, const wchar_t* str)
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
      StringCopy(valueName, lpValueName);
    }
    return RegSetValueExX(hKey, lpValueName ? valueName.c_str() : 0, dwType, lpData, cbData);
  }

  // RegQueryValueEx
  inline LONG RegQueryValueExStringX(HKEY hKey, const wchar_t* lpValueName, DWORD Reserved, DWORD dwType, std::wstring& str)
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
      StringCopy(valueName, lpValueName);
    }
    LONG r;
    if(ERROR_SUCCESS == (r = RegQueryValueExStringX(hKey, lpValueName ? valueName.c_str() : 0, Reserved, REG_SZ, strW)))
    {
      StringCopy(strX, strW);
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
      StringCopy(valueName, szValueName);
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
    StringCopy(outName, outNameW);
    return ret;
  }
  template<typename Char>
  inline LONG RegEnumKeyExX(HKEY hKey, DWORD dwIndex, std::basic_string<Char>& outName)
  {
    std::wstring outNameW;
    DWORD maxNameSize = 0;
    LONG ret = RegEnumKeyExX(hKey, dwIndex, outNameW, maxNameSize);
    StringCopy(outName, outNameW);
    return ret;
  }




#endif // _WIN32

  inline bool IsValidHandle(HANDLE a)
  {
    return (a != 0) && (a != INVALID_HANDLE_VALUE);
  }

  inline bool IsBadHandle(HANDLE a)
  {
    return !IsValidHandle(a);
  }

  template<typename Char>
  inline bool PathIsAbsolute(const std::basic_string<Char>& path)
  {
      bool r = false;
      if(path.length() >= 3)
      {
          std::basic_string<Char> sSearch = path.substr(1, 2);
          if(StringEquals(sSearch, ":/") || StringEquals(sSearch, ":\\"))
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
      nLastSlash = StringFindLastOf(path, "\\/");
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
    BlobTypes<Char>::PathBlob buf;
    //TCHAR* tsz = 0;
    DWORD dwRet = 0;
    bool r = false;

    dwRet = GetCurrentDirectoryW(s_cast<DWORD>(buf.Size()), buf.GetWritableBuffer());
    if(dwRet)
    {
      if(dwRet > s_cast<DWORD>(buf.size()))
      {
        // It needs more space.
        if(buf.Alloc(dwRet+2))
        {
          if(::GetCurrentDirectory(s_cast<DWORD>(buf.Size()), buf.GetWritableBuffer()))
          {
            out = buf.GetWritableBuffer();
            r = true;
          }

          buf.Free();
        }
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

#include "winapiimpl.h"

