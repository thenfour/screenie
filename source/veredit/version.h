

#pragma once

#include "binfile.h"


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
    Result Read(TBinary& f)
    {
      Result ret;
      if(!(ret = f.Read(length))) { ret.Prepend("Failed to read the 'length' field; "); goto Error; }
      if(!(ret = f.Read(valueLength))) { ret.Prepend("Failed to read the 'valueLength' field; "); goto Error; }
      if(!(ret = f.Read(type))) { ret.Prepend("Failed to read the 'type' field; "); goto Error; }
      if(!(ret = f.Read(key))) { ret.Prepend("Failed to read the 'key' field; "); goto Error; }
      // padding.
      f.Skip(GetPaddingLength(GetLengthWithoutPadding()));
      return ret.Succeed();
Error:
      return ret.Prepend("Error reading a version struct header; ");
    }
    template<typename TBinary>
    Result Write(TBinary& f)
    {
      Result ret;
      if(!(ret = f.Write(length))) { ret.Prepend("Failed to write the 'length' field; "); goto Error; }
      if(!(ret = f.Write(valueLength))) { ret.Prepend("Failed to write the 'valueLength' field; "); goto Error; }
      if(!(ret = f.Write(type))) { ret.Prepend("Failed to write the 'type' field; "); goto Error; }
      if(!(ret = f.Write(key))) { ret.Prepend("Failed to write the 'key' field; "); goto Error; }
      if(!(ret = f.WriteZeroBytes(GetPaddingLength(GetLengthWithoutPadding()))))
      {
        ret.Prepend("Failed to write the padding; ");
        goto Error;
      }
      return ret.Succeed();
Error:
      return ret.Prepend("Error writing a version struct header; ");
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
    Result Read(TBinary& f, VersionStructHeader& header)
    {
      Result ret;
      hdr = header;
      // read in the bytes of the data.  valueLength is the size in BYTES of the field.
      if(!values.Alloc(hdr.valueLength / sizeof(DWORD))) { ret.Fail("Error allocating memory."); goto Error; }
      if(!(ret = f.Read(values.GetBuffer(), values.Size()))) { ret.Prepend("Error reading Var DWORD values"); goto Error; }

      // padding.
      f.Skip(GetPaddingLength(GetLengthWithoutPadding()));

      return ret.Succeed();
Error:
      return ret.Prepend("Error reading a Var structure; ");
    }

    template<typename TBinary>
    Result Read(TBinary& f)
    {
      VersionStructHeader header;
      Result ret;
      if(!(ret = header.Read(f)))
      {
        return ret.Prepend("Error reading a Var structure; ");
      }
      return Read(f, header);
    }

    template<typename TBinary>
    Result Write(TBinary& f)
    {
      Result ret;
      hdr.valueLength = (WORD)GetValueLength();
      hdr.length = (WORD)GetLengthWithoutPadding();
      if(!(ret = hdr.Write(f))) { goto Error; }
      if(!(ret = f.Write(values.GetBuffer(), values.Size()))) { ret.Prepend("Error writing DWORD values; "); goto Error; }
      // padding
      if(!(ret = f.WriteZeroBytes(GetPaddingLength(GetLengthWithoutPadding())))) { ret.Prepend("Error writing padding; "); goto Error; }
      return ret.Succeed();
Error:
      return ret.Prepend("Error writing a Var structure; ");
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
    Result Read(TBinary& f, VersionStructHeader& header)
    {
      Result ret;
      hdr = header;
      if(hdr.valueLength != 0)
      {
        ret.Fail(Format("ValueLength is wrong.  Expected: 0, actual: %.")(hdr.valueLength));
        goto Error;
      }
      if(!(ret = value.Read(f))) { goto Error; }
      // padding.
      f.Skip(GetPaddingLength(GetLengthWithoutPadding()));
      return ret.Succeed();
Error:
      return ret.Prepend("Error reading a VarFileInfo structure; ");
    }

    template<typename TBinary>
    Result Read(TBinary& f)
    {
      VersionStructHeader header;
      Result ret;
      if(!(ret = header.Read(f)))
      {
        return ret.Prepend("Error reading a VarFileInfo structure; ");
      }
      return Read(f, header);
    }

    template<typename TBinary>
    Result Write(TBinary& f)
    {
      Result ret;
      hdr.valueLength = 0;// always 0
      hdr.length = (WORD)GetLengthWithoutPadding();
      if(!(ret = hdr.Write(f))) { goto Error; }
      if(!(ret = value.Write(f))) { goto Error; }
      // padding
      if(!(ret = f.WriteZeroBytes(GetPaddingLength(GetLengthWithoutPadding())))) { ret.Prepend("Error writing padding bytes; "); goto Error; }
      return ret.Succeed();
Error:
      return ret.Prepend("Error writing a VarFileInfo structure; ");
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

    String() { }
    String(PCTSTR) { }// don't ask! (this is a hack so i can make collection work

    template<typename TBinary>
    Result Read(TBinary& f, VersionStructHeader& header)
    {
      Result ret;
      hdr = header;
      // don't read anything if the string is null.
      value.clear();
      if(hdr.length > hdr.GetLengthWithPadding())
      {
        if(!(ret = f.Read(value)))
        {
          ret.Prepend("Error reading the string value; ");
          goto Error;
        }
      }
      // verify that hdr.valueLength is the value length in WORDs.
      if(hdr.valueLength != GetValueLengthChars())
      {
        ret.Fail(Format("The valueLength field was incorrect.  Expected %, it is %.").st(GetValueLengthChars()).st(hdr.valueLength));
        goto Error;
      }

      // padding.
      f.Skip(GetPaddingLength(GetLengthWithoutPadding()));
      return ret.Succeed();
Error:
      return ret.Prepend("Error reading a String structure; ");
    }

    template<typename TBinary>
    Result Read(TBinary& f)
    {
      VersionStructHeader header;
      Result ret;
      if(!(ret = header.Read(f)))
      {
        return ret.Prepend("Error reading a String structure; ");
      }
      return Read(f, header);
    }

    template<typename TBinary>
    Result Write(TBinary& f)
    {
      Result ret;
      hdr.length = (WORD)GetLengthWithoutPadding();
      hdr.valueLength = (WORD)GetValueLengthChars();
      if(!(ret = hdr.Write(f))) { goto Error; }
      if(!value.empty())
      {
        if(!(ret = f.Write(value)))
        {
          ret.Prepend("Error writing the string value; ");
          goto Error;
        }
      }
      // padding
      if(!(ret = f.WriteZeroBytes(GetPaddingLength(GetLengthWithoutPadding()))))
      {
        ret.Prepend("Error writing the padding bytes; ");
        goto Error;
      }
      return ret.Succeed();
Error:
      return ret.Prepend("Error writing a String structure; ");
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

    // not part of the disk structure.
    string name;

    Collection(PCTSTR szName) :
      name(szName)
    {
    }

    template<typename TBinary>
    Result Read(TBinary& f, VersionStructHeader& header)
    {
      Result ret;
      hdr = header;
      if(hdr.valueLength != 0)
      {
        ret.Fail(Format("ValueLength is wrong.  Expected: 0, actual: %.")(hdr.valueLength));
        goto Error;
      }
      size_t bytesLeft = hdr.length - hdr.GetLengthWithPadding();// valuelength is always 0
      while(bytesLeft)
      {
        items.push_back(Element("StringTable"));
        if(!(ret = items.back().Read(f)))
        {
          ret.Prepend("Error writing an item; ");
          goto Error;
        }
        bytesLeft -= items.back().GetLengthWithPadding();
      }
      // padding.
      f.Skip(GetPaddingLength(GetLengthWithoutPadding()));
      return ret.Succeed();
Error:
      return ret.Prepend(Format("Error reading a % structure; ")(name));
    }

    template<typename TBinary>
    Result Read(TBinary& f)
    {
      VersionStructHeader header;
      Result ret;
      if(!(ret = header.Read(f)))
      {
        return ret.Prepend(Format("Error reading a % structure; ")(name));
      }
      return Read(f, header);
    }

    template<typename TBinary>
    Result Write(TBinary& f)
    {
      Result ret;
      hdr.length = (WORD)GetLengthWithoutPadding();
      hdr.valueLength = 0;// StringTable and StringFileInfo always have this set to 0
      if(!(ret = hdr.Write(f))) { goto Error; }
      for(list<Element>::iterator it = items.begin(); it != items.end(); ++ it)
      {
        if(!(ret = it->Write(f)))
        {
          ret.Prepend("Error writing an item; ");
          goto Error;
        }
      }
      // padding
      f.WriteZeroBytes(GetPaddingLength(GetLengthWithoutPadding()));
      return ret.Succeed();
Error:
      return ret.Prepend(Format("Error writing a % structure; ")(name));
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
    Result Read(TBinary& f)
    {
      Result ret;
      if(!(ret = hdr.Read(f))) { ret.Prepend("Error reading initial version header; "); goto Error; }
      if(!(ret = f.Read(magicNumber))) { ret.Prepend("Error reading magic number; "); goto Error; }
      if(!(ret = f.Read(structVersion))) { ret.Prepend("Error reading struct version; "); goto Error; }
      if(!(ret = f.Read(fileVersionMS))) { ret.Prepend("Error reading file version MS; "); goto Error; }
      if(!(ret = f.Read(fileVersionLS))) { ret.Prepend("Error reading file version LS; "); goto Error; }
      if(!(ret = f.Read(productVersionMS))) { ret.Prepend("Error reading product version MS; "); goto Error; }
      if(!(ret = f.Read(productVersionLS))) { ret.Prepend("Error reading product version LS; "); goto Error; }
      if(!(ret = f.Read(fileFlagsMask))) { ret.Prepend("Error reading file flags mask; "); goto Error; }
      if(!(ret = f.Read(fileFlags))) { ret.Prepend("Error reading file flags; "); goto Error; }
      if(!(ret = f.Read(fileOS))) { ret.Prepend("Error reading file OS; "); goto Error; }
      if(!(ret = f.Read(fileType))) { ret.Prepend("Error reading file Type; "); goto Error; }
      if(!(ret = f.Read(fileSubType))) { ret.Prepend("Error reading file sub-type; "); goto Error; }
      if(!(ret = f.Read(fileDateMS))) { ret.Prepend("Error reading file date MS; "); goto Error; }
      if(!(ret = f.Read(fileDateLS))) { ret.Prepend("Error reading file date LS; "); goto Error; }

      size_t bytesLeft = (size_t)hdr.length - hdr.GetLengthWithPadding() - hdr.valueLength;

      // maybe do some verification
      if(magicNumber != 0xfeef04bd)
      {
        ret.Fail(Format("Magic number is incorrect. Expected 0xfeef04bd; it is 0x%").ul<16,8>(magicNumber));
        goto Error;
      }
      if(!StringEquals(hdr.key, "VS_VERSION_INFO"))
      {
        ret.Fail(Format("The root key is incorrect.  Expected \"VS_VERSION_INFO\"; it is %").qs(hdr.key));
        goto Error;
      }
      if(hdr.valueLength != GetValueLength())
      {
        ret.Fail(Format("The value length is incorrect.  Expected %; it is %").st(GetValueLength()).st(hdr.valueLength));
        goto Error;
      }

      // now we can have some other structs but may be of different types.  read the header, figure out which type, and dispatch.
      while(bytesLeft > VersionStructHeader::GetMinimumStructSize())
      {
        VersionStructHeader temp;
        if(!(ret = temp.Read(f)))
        {
          ret.Prepend("Error reading child header; ");
          goto Error;
        }
        if(StringEquals(temp.key, L"StringFileInfo"))
        {
          stringFileInfo.push_back(StringFileInfo("StringFileInfo"));
          if(!(ret = stringFileInfo.back().Read(f, temp))) { goto Error; }
          bytesLeft -= stringFileInfo.back().GetLengthWithPadding();
        }
        else if(StringEquals(temp.key, L"VarFileInfo"))
        {
          varFileInfo.push_back(VarFileInfo());
          if(!(ret = varFileInfo.back().Read(f, temp))) { goto Error; }
          bytesLeft -= varFileInfo.back().GetLengthWithPadding();
        }
        else
        {
          // invalid!
        }
      }

      return ret.Succeed();
Error:
      return ret.Prepend("Error reading the root version block; ");
    }

    template<typename TBinary>
    Result Write(TBinary& f)
    {
      Result ret;
      hdr.length = (WORD)GetLengthWithoutPadding() + (WORD)GetChildrenLength();
      hdr.valueLength = (WORD)GetValueLength();
      if(!(ret = hdr.Write(f))) { ret.Prepend("Error writing header; "); goto Error; }
      if(!(ret = f.Write(magicNumber))) { ret.Prepend("Error writing magic number; "); goto Error; }
      if(!(ret = f.Write(structVersion))) { ret.Prepend("Error writing struct version; "); goto Error; }
      if(!(ret = f.Write(fileVersionMS))) { ret.Prepend("Error writing file version MS; "); goto Error; }
      if(!(ret = f.Write(fileVersionLS))) { ret.Prepend("Error writing file version LS; "); goto Error; }
      if(!(ret = f.Write(productVersionMS))) { ret.Prepend("Error writing product version MS; "); goto Error; }
      if(!(ret = f.Write(productVersionLS))) { ret.Prepend("Error writing product version LS; "); goto Error; }
      if(!(ret = f.Write(fileFlagsMask))) { ret.Prepend("Error writing file flags mask; "); goto Error; }
      if(!(ret = f.Write(fileFlags))) { ret.Prepend("Error writing file flags; "); goto Error; }
      if(!(ret = f.Write(fileOS))) { ret.Prepend("Error writing file OS; "); goto Error; }
      if(!(ret = f.Write(fileType))) { ret.Prepend("Error writing file type; "); goto Error; }
      if(!(ret = f.Write(fileSubType))) { ret.Prepend("Error writing file sub-type; "); goto Error; }
      if(!(ret = f.Write(fileDateMS))) { ret.Prepend("Error writing file date MS; "); goto Error; }
      if(!(ret = f.Write(fileDateLS))) { ret.Prepend("Error writing file date LS; "); goto Error; }

      for(list<StringFileInfo>::iterator it = stringFileInfo.begin(); it != stringFileInfo.end(); ++ it)
      {
        if(!(ret = it->Write(f))) { goto Error; }
      }
      for(list<VarFileInfo>::iterator it = varFileInfo.begin(); it != varFileInfo.end(); ++ it)
      {
        if(!(ret = it->Write(f))) { goto Error; }
      }
      // padding
      if(!(ret = f.WriteZeroBytes(GetPaddingLength(GetLengthWithoutPadding()))))
      {
        ret.Prepend("Error writing padding bytes; ");
        goto Error;
      }
      return ret.Succeed();
Error:
      return ret.Prepend("Error writing the root version block; ");
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
