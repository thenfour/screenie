// veredit.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <windows.h>
#include "version.h"
#include "resource.h"
#include "binfile.h"

HINSTANCE g_hInstance = 0;

class Arg
{
public:
  string name;
  string value;
};

void Main(const string& file, vector<Arg> args)
{
}

int Usage()
{
  HRSRC h = FindResource(g_hInstance, MAKEINTRESOURCE(IDR_USAGE), _T("BIN"));
  DWORD size = SizeofResource(g_hInstance, h);
  HGLOBAL hGlobal = LoadResource(g_hInstance, h);
  const char* p = (const char*)LockResource(hGlobal);
  cout << string(p, size) << endl;
  return 1;
}

int _tmain(int argc, _TCHAR* argv[])
{
  //string file = "F:\\svn.screenie\\root\\bin-release\\screenie.exe";
  g_hInstance = GetModuleHandle(0);

  argc --;
  argv ++;

  if(argc < 2)
  {
    return Usage();
  }

  ////HMODULE h = LoadLibrary(file.c_str());
  ////g_indent = 0;
  ////EnumResourceTypes(h, EnumResTypeProc, 0);
  ////FreeLibrary(h);

  //DWORD wtf;
  //DWORD size = GetFileVersionInfoSize(file.c_str(), &wtf);
  //LibCC::Blob<BYTE> verdata(size);
  //GetFileVersionInfo(file.c_str(), 0, size, verdata.GetBuffer());
  //BinaryMemory mem(verdata.GetBuffer(), size);
  //mem.Skip(1);
  //Version::VersionResource res;
  //Result r = res.Read(mem);

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

	return 0;
}

