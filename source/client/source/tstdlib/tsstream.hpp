#ifndef _TSSTREAM_HPP_
# define _TSSTREAM_HPP_

// See tstd.hpp for more documentation
# include "tstd.hpp"

# include <sstream>

namespace tstd
{
	typedef std::basic_stringbuf<tchar_t> tstringbuf;
	typedef std::basic_istringstream<tchar_t> tistringstream;
	typedef std::basic_ostringstream<tchar_t> tostringstream;
	typedef std::basic_stringstream<tchar_t> tstringstream;
}

#endif 
