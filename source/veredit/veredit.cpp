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
        break;
      case Arg::DELETETYPE:
        break;
      case Arg::STRING:
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

    if(0 == UpdateResource(hResource, RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (PVOID)/*const_cast*/memFinal.GetBuffer(), (DWORD)memFinal.GetSize()))
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

    //DWORD wtf;
    //DWORD size = GetFileVersionInfoSize(file.c_str(), &wtf);
    //LibCC::Blob<BYTE> verdata(size);
    //GetFileVersionInfo(file.c_str(), 0, size, verdata.GetBuffer());
    //BinaryMemory mem(verdata.GetBuffer(), size);
    //mem.Skip(1);
    //Version::VersionResource res;
    //Result r = res.Read(mem);

    ////HMODULE h = LoadLibrary(file.c_str());
    ////g_indent = 0;
    ////EnumResourceTypes(h, EnumResTypeProc, 0);
    ////FreeLibrary(h);


    //BinaryMemory memout;
    //res.Write(memout);

    //BinaryFile f("i:\\ver", true);
    //f.Write(memout.GetBuffer(), memout.GetSize());

    //HANDLE hResource = BeginUpdateResource(file.c_str(), FALSE);
    //if (NULL != hResource)
    //{
    //  UINT uTemp;

    //  struct LANGANDCODEPAGE 
    //  {
    //    WORD wLanguage;
    //    WORD wCodePage;
    //  } translate;

    //  // get the language information
    //  if (VerQueryValue(verdata.GetBuffer(), _T("\\VarFileInfo\\Translation"), (LPVOID *) &translate, &uTemp) != FALSE)
    //  {
    //    // could probably just use LANG_NEUTRAL/SUBLANG_NEUTRAL
    //    if (UpdateResource(hResource, RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO), translate.wLanguage, lpBuffer, dwSize) != FALSE)
    //    {
    //      EndUpdateResource(hResource, FALSE);
    //    }
    //  }
    //}

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
  }
  CoUninitialize();
	return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{
  // use fake arguments.
  PTSTR argv__[] = 
    {
      "[name of exe]",
      "F:\\svn.screenie\\root\\source\\veredit\\test\\test.exe",
      "/delete",
      "nothing"
    };

  return main2(sizeof(argv__) / sizeof(PCTSTR), argv__);
}




