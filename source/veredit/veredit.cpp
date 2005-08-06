// veredit.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "version.h"
#include "misc.h"
#include "resource.h"
#include "binfile.h"
#include "versionxml.h"

HINSTANCE g_hInstance = 0;

int Usage()
{
  HRSRC h = FindResource(g_hInstance, MAKEINTRESOURCE(IDR_USAGE), _T("BIN"));
  DWORD size = SizeofResource(g_hInstance, h);
  HGLOBAL hGlobal = LoadResource(g_hInstance, h);
  const char* p = (const char*)LockResource(hGlobal);
  cout << string(p, size) << endl;
  return 1;
}

Result LoadVersionResource(const string& file, Version::VersionResource& resource)
{
  DWORD wtf;
  DWORD size = GetFileVersionInfoSize(file.c_str(), &wtf);
  LibCC::Blob<BYTE> bindata(size);
  GetFileVersionInfo(file.c_str(), 0, size, bindata.GetBuffer());
  BinaryMemory mem(bindata.GetBuffer(), size);
  return resource.Read(mem);
}

void DumpResourcesToDisk(const string& exe, const string& dump)
{
  DWORD wtf;
  DWORD size = GetFileVersionInfoSize(exe.c_str(), &wtf);
  LibCC::Blob<BYTE> bindata(size);
  GetFileVersionInfo(exe.c_str(), 0, size, bindata.GetBuffer());
  BinaryFile f(dump, true);
  f.Write(bindata.GetBuffer(), bindata.Size());
}

void ProcessDelete(Arg& a, Version::VersionResource& res)
{
  cout << "Deleting \"" << a.name << "\"...";
  int occurrences = 0;
  // enumerate StringFileInfo (really should only be 1 of these)
  for(list<Version::StringFileInfo>::iterator itSFI = res.stringFileInfo.begin(); itSFI != res.stringFileInfo.end(); ++ itSFI)
  {
    // enumerate string tables
    for(list<Version::StringTable>::iterator itST = itSFI->items.begin(); itST != itSFI->items.end(); ++ itST)
    {
      // enumerate strings
      for(list<Version::String>::iterator itS = itST->items.begin(); itS != itST->items.end();)
      {
        // make sure we don't invalidate this iterator.
        list<Version::String>::iterator itTemp = itS;
        ++ itS;// now this is safe.
        if(StringEquals(itTemp->hdr.key, a.name))
        {
          itST->items.erase(itTemp);
          occurrences ++;
        }
      }
    }
  }

  cout << "Done (" << occurrences << " occurrences deleted)." << endl;
}

Result ProcessString(Arg& a, Version::VersionResource& res)
{
  cout << "Setting string \"" << a.name << "\"...";
  int occurrences = 0;
  // enumerate StringFileInfo (really should only be 1 of these)
  for(list<Version::StringFileInfo>::iterator itSFI = res.stringFileInfo.begin(); itSFI != res.stringFileInfo.end(); ++ itSFI)
  {
    // enumerate string tables
    for(list<Version::StringTable>::iterator itST = itSFI->items.begin(); itST != itSFI->items.end(); ++ itST)
    {
      // enumerate strings
      for(list<Version::String>::iterator itS = itST->items.begin(); itS != itST->items.end(); ++ itS)
      {
        Version::String& s = *itS;
        if(StringEquals(s.hdr.key, a.name))
        {
          StringCopy(s.value, a.value);
          occurrences ++;
        }
      }
    }
  }

  if(occurrences)
  {
    cout << "Done (" << occurrences << " occurrences replaced)." << endl;
  }
  else
  {
    // make sure there's a string table to place it.
    if(!res.stringFileInfo.size())
    {
      cout << endl;
      return Result(false, "Error: No string file info block is in the version resource, so I can't add a new string to it.");;
    }
    if(!res.stringFileInfo.front().items.size())
    {
      cout << endl;
      return Result(false, "Error: No string tables were in the version resource, so I can't add a new string to it.");;
    }
    Version::StringTable& st = res.stringFileInfo.front().items.front();
    Version::String s;
    StringCopy(s.value, a.value);
    StringCopy(s.hdr.key, a.name);
    s.hdr.type = 1;
    st.items.push_back(s);

    cout << "Done (1 new string added)." << endl;
  }

  return Result(true);
}


Result ProcessFixed(Arg& a, Version::VersionResource& res)
{
  cout << "Setting fixed value \"" << a.name << "\"...";
  Result r;
  DWORD value;
  if(!(r = ParseDWORDString(a.value, value)))
  {
    cout << endl;
    return r;
  }

  if(a.name == "dwFileVersionMS")
  {
    res.fileVersionMS = value;
  }
  else if(a.name == "dwFileVersionLS")
  {
    res.fileVersionLS = value;
  }
  else if(a.name == "dwProductVersionMS")
  {
    res.productVersionMS = value;
  }
  else if(a.name == "dwProductVersionLS")
  {
    res.productVersionLS = value;
  }
  else if(a.name == "dwFileFlagsMask")
  {
    res.fileFlagsMask = value;
  }
  else if(a.name == "dwFileFlags")
  {
    res.fileFlags = value;
  }
  else if(a.name == "dwFileOS")
  {
    res.fileOS = value;
  }
  else if(a.name == "dwFileType")
  {
    res.fileType = value;
  }
  else if(a.name == "dwFileSubtype")
  {
    res.fileSubType = value;
  }
  else if(a.name == "dwFileDateMS")
  {
    res.fileDateMS = value;
  }
  else if(a.name == "dwFileDateLS")
  {
    res.fileDateLS = value;
  }
  else
  {
    cout << endl;
    return r.Fail(Format("Unrecognized fixed value name: %").qs(a.name));
  }

  cout << "Done (1 value updated)." << endl;

  return r.Succeed();
}

int main2(int argc, _TCHAR** argv)
{
  CoInitialize(0);
  {
    //string file = "F:\\svn.screenie\\root\\bin-release\\screenie.exe";
    g_hInstance = GetModuleHandle(0);

    argc --;
    argv ++;

    if(argc < 2)
    {
      return Usage();
    }

    // get the filename
    string file = argv[0];
    if(!PathFileExists(file.c_str()))
    {
      cout << "File not found: " << file << endl;
      return 1;
    }

    // parse the rest of the command line args
    argc --;
    argv ++;

    vector<Arg> args;

    Result res;

    for(int i = 0; i < argc; i ++)
    {
      string arg = argv[i];

      if(StringEquals(arg, "/xml"))
      {
        i ++;
        if(i >= argc)
        {
          cout << "/string command line argument is missing its operand." << endl;
          return 1;
        }
        // this will convert the xml file into a list of arguments.
        VersionXMLFile f;
        if(!(res = f.Read(argv[i])))
        {
          cout << res.str() << endl;
          return 1;
        }
        // merge the lists together.
        for(vector<Arg>::iterator it = f.args.begin(); it != f.args.end(); ++ it)
        {
          args.push_back(*it);
        }
      }
      else if(StringEquals(arg, "/string"))
      {
        i ++;
        if(i >= argc)
        {
          cout << "/string command line argument is missing its operand." << endl;
          return 1;
        }
        Arg temp;
        if(!(res = ParseNameValueString(argv[i], temp.name, temp.value)))
        {
          res.Prepend("/string command line argument invalid; ");
          cout << res.str() << endl;
          return 1;
        }
        temp.source = "/string command line argument";
        temp.type = Arg::STRING;
        args.push_back(temp);
      }
      else if(StringEquals(arg, "/delete"))
      {
        i ++;
        if(i >= argc)
        {
          cout << "/delete command line argument is missing its operand." << endl;
          return 1;
        }
        Arg temp;
        temp.source = "/delete command line argument";
        temp.type = Arg::DELETETYPE;
        temp.name = argv[i];
        args.push_back(temp);
      }
      else if(StringEquals(arg, "/fixed"))
      {
        i ++;
        if(i >= argc)
        {
          cout << "/fixed command line argument is missing its operand." << endl;
          return 1;
        }
        Arg temp;
        if(!(res = ParseNameValueString(argv[i], temp.name, temp.value)))
        {
          res.Prepend("/fixed command line argument invalid; ");
          cout << res.str() << endl;
          return 1;
        }
        temp.source = "/fixed command line argument";
        temp.type = Arg::FIXED;
        args.push_back(temp);
      }
      else
      {
        cout << "Unrecognized command line switch: " << arg << endl;
        return 1;
      }
    }

    // open version info.
    Version::VersionResource versionResource;

    if(!(res = LoadVersionResource(file, versionResource)))
    {
      res.Prepend(Format("Failed to read version info from %; ").qs(file));
      cout << res.str() << endl;
      return 1;
    }

    // process the arguments sequentially.
    for(vector<Arg>::iterator it = args.begin(); it != args.end(); ++ it)
    {
      Arg& a(*it);
      switch(a.type)
      {
      case Arg::FIXED:
        if(!(res = ProcessFixed(a, versionResource)))
        {
          cout << res.str() << endl;
          return 1;
        }
        break;
      case Arg::DELETETYPE:
        ProcessDelete(a, versionResource);
        break;
      case Arg::STRING:
        if(!(res = ProcessString(a, versionResource)))
        {
          cout << res.str() << endl;
          return 1;
        }
        break;
      }
    }

    // write it back out in memory
    BinaryMemory memFinal;
    if(!(res = versionResource.Write(memFinal)))
    {
      res.Prepend(Format("Failed to generate version info; "));
      cout << res.str() << endl;
      return 1;
    }

    // update the resource.
    HANDLE hResource = BeginUpdateResource(file.c_str(), FALSE);
    if(NULL == hResource)
    {
      cout << Format("Error updating resources for %. BeginUpdateResource returned %").qs(file).gle().Str() << endl;
      return 1;
    }

    if(0 == UpdateResource(hResource, RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO), versionResource.GetLanguage(), (PVOID)/*const_cast*/memFinal.GetBuffer(), (DWORD)memFinal.GetSize()))
    {
      cout << Format("Error updating resources for %. UpdateResource returned %").qs(file).gle().Str() << endl;
      return 1;
    }

    if(0 == EndUpdateResource(hResource, FALSE))
    {
      cout << Format("Error updating resources for %. EndUpdateResource returned %").qs(file).gle().Str() << endl;
      return 1;
    }

    cout << "Version updated successfully." << endl;
  }
  CoUninitialize();
	return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{
#if 0
  // use fake arguments.
  PTSTR argv__[] = 
    {
      "[name of exe]",
      "F:\\svn.screenie\\root\\distro\\1\\screenie.exe",
      "/xml",
      "F:\\svn.screenie\\root\\distro\\1\\ver_out.xml"
    };
 
  //"%outdir%\screenie.exe" /xml "%outdir%\ver_out.xml" /string RegisteredTo="%registrant%"
 
  // DumpResourcesToDisk(argv__[1], "F:\\svn.screenie\\root\\source\\veredit\\test\\test2.bin");

  return main2(sizeof(argv__) / sizeof(PCTSTR), argv__);
#else
  return main2(argc, argv);
#endif
}




