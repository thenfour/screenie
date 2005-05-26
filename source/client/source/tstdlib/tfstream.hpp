#ifndef _TFSTREAM_HPP_
# define _TFSTREAM_HPP_

// See tstd.hpp for more documentation
# include "tstd.hpp"

# include <fstream>

namespace tstd
{
	typedef std::basic_filebuf<tchar_t> tfilebuf;
	typedef std::basic_ifstream<tchar_t> tifstream;
	typedef std::basic_ofstream<tchar_t> tofstream;
	typedef std::basic_fstream<tchar_t> tfstream;
}

#endif 
