#ifndef _TIOSFWD_HPP_
# define _TIOSFWD_HPP_

// See tstd.hpp for more documentation
# include "tstd.hpp"

# include <iosfwd>

namespace tstd
{
	typedef std::basic_ios<tchar_t> tios;
	typedef std::basic_streambuf<tchar_t> tstreambuf;
	typedef std::basic_istream<tchar_t> tistream;
	typedef std::basic_ostream<tchar_t> tostream;
	typedef std::basic_iostream<tchar_t> tiostream;
	
	typedef std::basic_stringbuf<tchar_t> tstringbuf;
	typedef std::basic_istringstream<tchar_t> tistringstream;
	typedef std::basic_ostringstream<tchar_t> tostringstream;
	typedef std::basic_stringstream<tchar_t> tstringstream;
	
	typedef std::basic_filebuf<tchar_t> tfilebuf;
	typedef std::basic_ifstream<tchar_t> tifstream;
	typedef std::basic_ofstream<tchar_t> tofstream;
	typedef std::basic_fstream<tchar_t> tfstream;
}

#endif