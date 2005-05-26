#ifndef _TIOSTREAM_HPP_
# define _TIOSTREAM_HPP_

// See tstd.hpp for more documentation
# include "tstd.hpp"
# include "tios.hpp"
# include "tstreambuf.hpp"
# include "tistream.hpp"
# include "tostream.hpp"

# include <iostream>

namespace tstd
{
	typedef std::basic_iostream<tchar_t> tiostream;
	
# if defined(_UNICODE)
	tistream &tcin  = std::wcin;
	tostream &tcout = std::wcout;
	tostream &tcerr = std::wcerr;
	tostream &tclog = std::wclog;
# else
	tistream &tcin  = std::cin;
	tostream &tcout = std::cout;
	tostream &tcerr = std::cerr;
	tostream &tclog = std::clog;
# endif
}

#endif
