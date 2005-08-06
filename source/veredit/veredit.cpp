// veredit.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include "..\client\source\libcc\ccstr.h"
#include "..\client\source\libcc\result.h"
#include "..\client\source\libcc\blob.h"
#include <list>
#include <iostream>
#include "binfile.h"

#pragma comment(lib, "version.lib")

using namespace std;


//int g_indent = 0;

//BOOL CALLBACK EnumResNameProc(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam)
//{
//  string name;
//  if(IS_INTRESOURCE(lpszName))
//  {
//    name = LibCC::Format().ul((DWORD)lpszName).Str();
//  }
//  else
//  {
//    name = lpszName;
//  }
//  cout << LibCC::Format("%% % %").c(' ', g_indent).p(hModule).qs(lpszType).qs(name).Str() << endl;
//  return TRUE;
//}
//
//
//BOOL CALLBACK EnumResTypeProc(HMODULE hModule, LPTSTR lpszType, LONG_PTR lParam)
//{
//  cout << LibCC::Format("%% % %").c(' ', g_indent).p(hModule).qs(lpszType).Str() << endl;
//  g_indent += 2;
//  EnumResourceNames(hModule, lpszType, EnumResNameProc, 0);
//  g_indent -= 2;
//  return TRUE;
//}


namespace Version
{
  size_t GetPaddingLength(size_t len)
  {
    size_t remainder = len % sizeof(DWORD);
    size_t ret = 0;
    if(remainder)
    {
      ret = sizeof(DWORD) - remainder;
    }
    return ret;
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  class VersionStructHeader
  {
  public:
    WORD length;
    WORD valueLength;
    WORD type;
    wstring key;

    template<typename TBinary>
    bool Read(TBinary& f)
    {
      f.Read(length);
      f.Read(valueLength);
      f.Read(type);
      f.Read(key);
      // padding.
      f.Skip(GetPaddingLength(GetLengthWithoutPadding()));
      return true;
    }
    template<typename TBinary>
    bool Write(TBinary& f)
    {
      f.Write(length);
      f.Write(valueLength);
      f.Write(type);
      f.Write(key);
      f.WriteZeroBytes(GetPaddingLength(GetLengthWithoutPadding()));
      return true;
    }
    size_t GetLengthWithoutPadding() const
    {
      return ((key.length() + 1) * sizeof(WCHAR)) + sizeof(length) + sizeof(valueLength) + sizeof(type);
    }
    size_t GetLengthWithPadding() const
    {
      return GetLengthWithoutPadding() + GetPaddingLength(GetLengthWithoutPadding());
    }
    static size_t GetMinimumStructSize()
    {
      // 2 char key would be probably small as small as it ever could get
      return (2 * sizeof(WCHAR)) + (sizeof(WORD) * 3);
    }
  };

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  class Var
  {
  public:
    VersionStructHeader hdr;
    LibCC::Blob<DWORD> values;

    Var()
    {
    }
    
    Var(const Var& rhs)
    {
      hdr = rhs.hdr;
      values.Assign(rhs.values);
    }

    template<typename TBinary>
    bool Read(TBinary& f, VersionStructHeader& header)
    {
      hdr = header;
      // read in the bytes of the data.  valueLength is the size in BYTES of the field.
      values.Alloc(hdr.valueLength / sizeof(DWORD));
      f.Read(values.GetBuffer(), values.Size());

      // padding.
      f.Skip(GetPaddingLength(GetLengthWithoutPadding()));
      return true;
    }

    template<typename TBinary>
    bool Read(TBinary& f)
    {
      VersionStructHeader header;
      header.Read(f);
      return Read(f, header);
    }

    template<typename TBinary>
    bool Write(TBinary& f)
    {
      hdr.valueLength = (WORD)GetValueLength();// always 0
      hdr.length = (WORD)GetLengthWithoutPadding();
      hdr.Write(f);
      f.Write(values.GetBuffer(), values.Size());
      // padding
      f.WriteZeroBytes(GetPaddingLength(GetLengthWithoutPadding()));
      return true;
    }

    size_t GetValueLength() const
    {
      return values.Size() * sizeof(DWORD);
    }
    size_t GetLengthWithoutPadding() const
    {
      return hdr.GetLengthWithPadding() + GetValueLength();
    }
    size_t GetLengthWithPadding() const
    {
      return GetLengthWithoutPadding() + GetPaddingLength(GetLengthWithoutPadding());
    }
  };

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  class VarFileInfo
  {
  public:
    VersionStructHeader hdr;
    Var value;

    template<typename TBinary>
    bool Read(TBinary& f, VersionStructHeader& header)
    {
      hdr = header;
      value.Read(f);
      // padding.
      f.Skip(GetPaddingLength(GetLengthWithoutPadding()));
      return true;
    }

    template<typename TBinary>
    bool Read(TBinary& f)
    {
      VersionStructHeader header;
      header.Read(f);
      return Read(f, header);
    }

    template<typename TBinary>
    bool Write(TBinary& f)
    {
      hdr.valueLength = 0;// always 0
      hdr.length = (WORD)GetLengthWithoutPadding();
      hdr.Write(f);
      value.Write(f);
      // padding
      f.WriteZeroBytes(GetPaddingLength(GetLengthWithoutPadding()));
      return true;
    }
    size_t GetValueLength() const
    {
      return value.GetLengthWithPadding();
    }
    size_t GetLengthWithoutPadding() const
    {
      return hdr.GetLengthWithPadding() + GetValueLength();
    }
    size_t GetLengthWithPadding() const
    {
      return GetLengthWithoutPadding() + GetPaddingLength(GetLengthWithoutPadding());
    }
  };

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  class String
  {
  public:
    VersionStructHeader hdr;
    wstring value;

    template<typename TBinary>
    bool Read(TBinary& f, VersionStructHeader& header)
    {
      hdr = header;
      // don't read anything if the string is null.
      value.clear();
      if(hdr.length > hdr.GetLengthWithPadding())
      {
        f.Read(value);
      }
      // verify that hdr.valueLength is the value length in WORDs.

      // padding.
      f.Skip(GetPaddingLength(GetLengthWithoutPadding()));
      return true;
    }

    template<typename TBinary>
    bool Read(TBinary& f)
    {
      VersionStructHeader header;
      header.Read(f);
      return Read(f, header);
    }

    template<typename TBinary>
    bool Write(TBinary& f)
    {
      hdr.length = (WORD)GetLengthWithoutPadding();
      hdr.valueLength = (WORD)GetValueLengthChars();
      hdr.Write(f);
      if(!value.empty())
      {
        f.Write(value);
      }
      // padding
      f.WriteZeroBytes(GetPaddingLength(GetLengthWithoutPadding()));
      return true;
    }

    size_t GetValueLengthChars() const
    {
      if(value.empty()) return 0;
      return (value.length() + 1);// length, in WORDs
    }
    size_t GetValueLengthBytes() const
    {
      return GetValueLengthChars() * sizeof(WCHAR);
    }
    size_t GetLengthWithoutPadding() const
    {
      return hdr.GetLengthWithPadding() + GetValueLengthBytes();
    }
    size_t GetLengthWithPadding() const
    {
      return GetLengthWithoutPadding() + GetPaddingLength(GetLengthWithoutPadding());
    }
  };

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  template<typename Element>
  class Collection
  {
  public:
    VersionStructHeader hdr;
    list<Element> items;

    template<typename TBinary>
    bool Read(TBinary& f, VersionStructHeader& header)
    {
      // assume the header has already been read for me
      hdr = header;
      size_t bytesLeft = hdr.length - hdr.GetLengthWithPadding();// valuelength is always 0
      while(bytesLeft)
      {
        items.push_back(Element());
        items.back().Read(f);
        bytesLeft -= items.back().GetLengthWithPadding();
      }
      // padding.
      f.Skip(GetPaddingLength(GetLengthWithoutPadding()));
      return true;
    }

    template<typename TBinary>
    bool Read(TBinary& f)
    {
      VersionStructHeader header;
      header.Read(f);
      return Read(f, header);
    }

    template<typename TBinary>
    bool Write(TBinary& f)
    {
      hdr.length = (WORD)GetLengthWithoutPadding();
      hdr.valueLength = 0;// StringTable and StringFileInfo always have this set to 0
      hdr.Write(f);
      for(list<Element>::iterator it = items.begin(); it != items.end(); ++ it)
      {
        it->Write(f);
      }
      // padding
      f.WriteZeroBytes(GetPaddingLength(GetLengthWithoutPadding()));
      return true;
    }

    size_t GetValueLength() const
    {
      size_t ret = 0;
      for(list<Element>::const_iterator it = items.begin(); it != items.end(); ++ it)
      {
        ret += it->GetLengthWithPadding();
      }
      return ret;
    }
    size_t GetLengthWithoutPadding() const
    {
      return hdr.GetLengthWithPadding() + GetValueLength();
    }
    size_t GetLengthWithPadding() const
    {
      return GetLengthWithoutPadding() + GetPaddingLength(GetLengthWithoutPadding());
    }
  };
  typedef Collection<String> StringTable;
  typedef Collection<StringTable> StringFileInfo;

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  class VersionResource
  {
  public:
    VersionStructHeader hdr;
    DWORD magicNumber;
    DWORD structVersion;
    DWORD fileVersionMS;
    DWORD fileVersionLS;
    DWORD productVersionMS;
    DWORD productVersionLS;
    DWORD fileFlagsMask;
    DWORD fileFlags;
    DWORD fileOS;
    DWORD fileType;
    DWORD fileSubType;
    DWORD fileDateMS;
    DWORD fileDateLS;

    list<StringFileInfo> stringFileInfo;
    list<VarFileInfo> varFileInfo;

    template<typename TBinary>
    bool Read(TBinary& f)
    {
      hdr.Read(f);
      f.Read(magicNumber);
      f.Read(structVersion);
      f.Read(fileVersionMS);
      f.Read(fileVersionLS);
      f.Read(productVersionMS);
      f.Read(productVersionLS);
      f.Read(fileFlagsMask);
      f.Read(fileFlags);
      f.Read(fileOS);
      f.Read(fileType);
      f.Read(fileSubType);
      f.Read(fileDateMS);
      f.Read(fileDateLS);

      size_t bytesLeft = (size_t)hdr.length - hdr.GetLengthWithPadding() - hdr.valueLength;

      // now we can have some other structs but may be of different types.  read the header, figure out which type, and dispatch.
      while(bytesLeft > VersionStructHeader::GetMinimumStructSize())
      {
        VersionStructHeader temp;
        temp.Read(f);
        if(LibCC::StringEquals(temp.key, L"StringFileInfo"))
        {
          stringFileInfo.push_back(StringFileInfo());
          stringFileInfo.back().Read(f, temp);
          bytesLeft -= stringFileInfo.back().GetLengthWithPadding();
        }
        else if(LibCC::StringEquals(temp.key, L"VarFileInfo"))
        {
          varFileInfo.push_back(VarFileInfo());
          varFileInfo.back().Read(f, temp);
          bytesLeft -= varFileInfo.back().GetLengthWithPadding();
        }
        else
        {
          // invalid!
        }
      }

      return true;
    }

    template<typename TBinary>
    bool Write(TBinary& f)
    {
      hdr.length = (WORD)GetLengthWithoutPadding() + (WORD)GetChildrenLength();
      hdr.valueLength = (WORD)GetValueLength();
      hdr.Write(f);
      f.Write(magicNumber);
      f.Write(structVersion);
      f.Write(fileVersionMS);
      f.Write(fileVersionLS);
      f.Write(productVersionMS);
      f.Write(productVersionLS);
      f.Write(fileFlagsMask);
      f.Write(fileFlags);
      f.Write(fileOS);
      f.Write(fileType);
      f.Write(fileSubType);
      f.Write(fileDateMS);
      f.Write(fileDateLS);

      for(list<StringFileInfo>::iterator it = stringFileInfo.begin(); it != stringFileInfo.end(); ++ it)
      {
        it->Write(f);
      }
      for(list<VarFileInfo>::iterator it = varFileInfo.begin(); it != varFileInfo.end(); ++ it)
      {
        it->Write(f);
      }
      // padding
      f.WriteZeroBytes(GetPaddingLength(GetLengthWithoutPadding()));
      return true;
    }

    size_t GetValueLength() const
    {
      return 13 * sizeof(DWORD);
    }

    size_t GetChildrenLength() const
    {
      size_t ret = 0;
      for(list<StringFileInfo>::const_iterator it = stringFileInfo.begin(); it != stringFileInfo.end(); ++ it)
      {
        ret += it->GetLengthWithPadding();
      }
      for(list<VarFileInfo>::const_iterator it = varFileInfo.begin(); it != varFileInfo.end(); ++ it)
      {
        ret += it->GetLengthWithPadding();
      }
      return ret;
    }

    size_t GetLengthWithoutPadding() const
    {
      return hdr.GetLengthWithPadding() + GetValueLength();
    }
  };
}

/*
  veredit.exe [exe] /i ver.ini /s RegisteredTo="Carl Corcoran"

  ver.ini:

  <StringFileInfo
    FileVersion=""
    Comments=""
    CompanyName="Carl Corcoran and Roger Clark"
    FileVersion="4, 2, 1, 0"
    LegalCopyright="(c)2005 Carl Corcoran and Roger Clark.  All rights reserved."
    ProductVersion="4, 2, 1, 0"
  />
  <FixedFileInfo
    dwSignature=0xffffffff
    dwStrucVersion=0xffffffff
    dwFileVersionMS=0xffffffff
    dwFileVersionLS=0xffffffff
    dwProductVersionMS=0xffffffff
    dwProductVersionLS=0xffffffff
    dwFileFlagsMask=0xffffffff
    dwFileFlags=0xffffffff
    dwFileOS=0xffffffff
    dwFileType=0xffffffff
    dwFileSubtype=0xffffffff
    dwFileDateMS=0xffffffff
    dwFileDateLS=0xffffffff
  />
  [00 00 00 00]
  0xffeeddcc
  0xaabb
  123

*/
int _tmain(int argc, _TCHAR* argv[])
{
  string file = "F:\\svn.screenie\\root\\bin-release\\screenie.exe";

  //HMODULE h = LoadLibrary(file.c_str());
  //g_indent = 0;
  //EnumResourceTypes(h, EnumResTypeProc, 0);
  //FreeLibrary(h);

  DWORD wtf;
  DWORD size = GetFileVersionInfoSize(file.c_str(), &wtf);
  LibCC::Blob<BYTE> verdata(size);
  GetFileVersionInfo(file.c_str(), 0, size, verdata.GetBuffer());
  BinaryMemory mem(verdata.GetBuffer(), size);
  Version::VersionResource res;
  res.Read(mem);

  BinaryMemory memout;
  res.Write(memout);

  BinaryFile f("i:\\ver", true);
  f.Write(memout.GetBuffer(), memout.GetSize());

  HANDLE hResource = BeginUpdateResource(file.c_str(), FALSE);
  if (NULL != hResource)
  {
    UINT uTemp;

    struct LANGANDCODEPAGE 
    {
      WORD wLanguage;
      WORD wCodePage;
    } translate;

    // get the language information
    if (VerQueryValue(verdata.GetBuffer(), _T("\\VarFileInfo\\Translation"), (LPVOID *) &translate, &uTemp) != FALSE)
    {
      // could probably just use LANG_NEUTRAL/SUBLANG_NEUTRAL
      if (UpdateResource(hResource, RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO), translate.wLanguage, lpBuffer, dwSize) != FALSE)
      {
        EndUpdateResource(hResource, FALSE);
      }
    }
  }

  //// dump it.
  //HANDLE hFile = CreateFile("crap", GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
  //WriteFile(hFile, verdata.GetBuffer(), verdata.Size(), &wtf, 0);
  //CloseHandle(hFile);

  //VS_VERSION_INFO a;

  //MSG msg;
  //while(GetMessage(&msg, 0, 0, 0))
  //{
  //  TranslateMessage(&msg);
  //  DispatchMessage(&msg);
  //}

	return 0;
}

