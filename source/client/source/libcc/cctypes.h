

#pragma once


/*
  this is the lowest-level header.  no dependencies except system stuff (windows.h)
*/

#include <tchar.h>

#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>

#include <stdexcept>
#include <string>
#include <algorithm>
#include <functional>
#include <iostream>
#include <vector>
#include <list>
#include <sstream>

#include <ctype.h>
#include <math.h>
#include <malloc.h>


namespace LibCC
{

#define scope_enum(name) \
	struct name \
	{ \
		enum _EnumType##name \

#define end_scope_enum(name) \
		} _m_val; \
		name(){} \
		name(const name& r) : _m_val(r._m_val) { } \
		name(int n) : _m_val((_EnumType##name)n) { } \
		name(_EnumType##name r) : _m_val(r) { } \
		operator int() const { return _m_val; } \
		name& operator = (_EnumType##name r) { _m_val = r; return *this; }


  typedef std::basic_string<TCHAR> _tstring;
  typedef std::basic_stringstream<TCHAR> _tstringstream;
  typedef std::basic_istringstream<TCHAR> _tistringstream;
  typedef std::basic_ostringstream<TCHAR> _tostringstream;
  typedef std::basic_iostream<TCHAR> _tiostream;
  typedef std::basic_istream<TCHAR> _tistream;
  typedef std::basic_ostream<TCHAR> _tostream;
  typedef std::basic_streambuf<TCHAR> _tstreambuf;
  typedef std::basic_stringbuf<TCHAR> _tstringbuf;
  typedef std::basic_filebuf<TCHAR> _tfilebuf;
  typedef std::basic_ifstream<TCHAR> _tifstream;
  typedef std::basic_ofstream<TCHAR> _tofstream;
  typedef std::basic_fstream<TCHAR> _tfstream;
  typedef std::basic_ios<TCHAR> _tios;
  #ifdef _UNICODE
  #   define _tcin                   std::wcin
  #   define _tcout                  std::wcout
  #   define _tcerr                  std::wcerr
  #   define _tclog                  std::wclog
  #else
  #   define _tcin                   std::cin
  #   define _tcout                  std::cout
  #   define _tcerr                  std::cerr
  #   define _tclog                  std::clog
  #endif

}
