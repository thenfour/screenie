// textreplace.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
#include <iostream>
#include <string>
#include "..\client\source\libcc\ccstr.h"
#include "..\client\source\libcc\result.h"
#include "..\client\source\libcc\blob.h"

using namespace std;
using namespace LibCC;

// splits a string at the first equal sign.
Result ParseNameValueString(const string& in, string& name, string& value)
{
  string::size_type nEq = in.find('=');
  if(nEq == string::npos)
  {
    return Result(false, "Invalid value... no equal sign was found.");
  }
  name = in.substr(0, nEq);
  value = in.c_str() + nEq + 1;// i'm not sure you can substr() at the null-term, so i'll do it this way where it's guaranteed to be safe
  return Result(true);
}

bool ReadTextFile(const string& file, string& out)
{
  HANDLE h = CreateFile(file.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
  if(IsBadHandle(h))
  {
    return false;
  }
  // allocate the mem.
  LibCC::Blob<BYTE> buf(GetFileSize(h, 0));
  // read it in.
  DWORD br;
  ReadFile(h, buf.GetBuffer(), (DWORD)buf.Size(), &br, 0);
  out.assign((PCSTR)buf.GetBuffer(), buf.Size());
  CloseHandle(h);
  return br == buf.Size();
}

bool WriteTextFile(const string& file, const string& text)
{
  HANDLE h = CreateFile(file.c_str(), GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
  if(IsBadHandle(h))
  {
    return false;
  }
  DWORD br;
  WriteFile(h, text.c_str(), (DWORD)text.size(), &br, 0);
  CloseHandle(h);
  return br == text.size();
}

int _tmain(int argc, _TCHAR* argv[])
{
  argc --;
  argv ++;

  if(argc < 2)
  {
    cout << "usage: textreplace.exe filename.txt find=replace find_more=replace_more ..." << endl;
    cout << "note: only works on ascii text files." << endl;
    return 1;
  }

  // get the file
  string file = argv[0];

  argc --;
  argv ++;

  // get the replace arguments
  vector<pair<string, string> > args;

  for(int i = 0; i < argc; i ++)
  {
    pair<string, string> arg;
    if(!ParseNameValueString(argv[i], arg.first, arg.second))
    {
      cout << "Error parsing command line; make sure you are surrounding the right stuff with quotes and everything." << endl;
      return 1;
    }
    args.push_back(arg);
  }

  // read
  string text;
  if(!ReadTextFile(file, text))
  {
    cout << "Error opening " << file << endl;
    return 1;
  }

  // ok now do the replace.
  for(vector<pair<string, string> >::iterator it = args.begin(); it != args.end(); ++ it)
  {
    text = LibCC::StringReplace(text.c_str(), it->first.c_str(), it->second.c_str());
  }

  // write
  if(!WriteTextFile(file, text))
  {
    cout << "Error modifying " << file << endl;
    return 1;
  }

	return 0;
}

