#ifndef _TSTRING_HPP_
# define _TSTRING_HPP_

// See tstd.hpp for more documentation
# include "tstd.hpp"

# include <vector> // for buffer space
# include <typeinfo> // for bad_cast
# include <locale> // for codecvt
# include <string>

namespace tstd
{
	typedef std::basic_string<tchar_t> tstring;

	// This function takes a string (either wide or ansi) and 
	// converts it to TCHAR. Ideally, this should use codecvt
	// in the standard library, but we're in windows here.
	// Not only that, but codecvt has such a massively broken
	// interface as far as I can tell it's probably not worth it.
	namespace convert_impl
	{
# if !defined(_NOWCHAR)

		// the opposite_type template is specialized for both string and wstring
		// so that the normal functions below can be automatically generated as templates
		// without having to define every combination. Other conversions for platform
		// specific char types could theoretically also be added.
		template <typename tString>
		struct opposite_type;
		// specialize for tString being the same as std::string
		template <>
		struct opposite_type<std::string>
		{
			typedef std::wstring type;
		};
		// specialize for tString being the same as std::wstring
		template <>
		struct opposite_type<std::wstring>
		{
			typedef std::string type;
		};

		// we specialize the converter class on char and wchar_t so that the functions
		// below can, again, automatically generate the correct conversion without defining all
		// permutations. They now use the std::codecvt facet of the std::locale class passed
		// in to define their conversion behaviour.
		template <typename tTo>
		struct converter;
		template <>
		struct converter<wchar_t>
		{
			static std::wstring convert(const std::string &from, const std::locale &loc = std::locale())
			{
				if (from.length() == 0)
					return L"";
					
				typedef std::codecvt<wchar_t, char, std::mbstate_t> codecvt;
				const codecvt &cvt = std::use_facet<codecvt>(loc);
				
				// each wchar_t can be up to cvt.max_length() chars, so we make our buffer
				// len / max, and add an extra max for rounding.
				// The vector is used as a buffer to hold the string. While the &* trick
				// is obscure, it is believed to be correct, and it is also exception safe.

				// unfortunately, this doesn't actually work correctly.
				// std::size_t maxlen = from.length() / cvt.max_length() + cvt.max_length();

				std::size_t maxlen = from.length() * 2;
				std::vector<wchar_t> arr(maxlen, L' ');

				// Setting up the state information needed by the codecvt facet.
				std::mbstate_t state = std::mbstate_t();
				const char *fromnext = NULL;
				wchar_t *tonext = NULL;
				codecvt::result r = cvt.in(state,
				                           from.data(), from.data() + from.length(), fromnext,
				                           &*arr.begin(), &*arr.end(), tonext);
				
				// This could probably be made smarter, but it's hard to say what the smart thing to
				// do is given a partial or error response. Given use cases, a more robust interface could
				// probably be derived.
				if (r != codecvt::ok)
					throw std::bad_cast();
					
				return std::wstring(&*arr.begin(), tonext);
			}
		};
		template <>
		struct converter<char>
		{
			static std::string convert(const std::wstring &from, const std::locale &loc = std::locale())
			{
				if (from.length() == 0)
					return "";
					
				typedef std::codecvt<wchar_t, char, std::mbstate_t> codecvt;
				const codecvt &cvt = std::use_facet<codecvt>(loc);
				
				// Each wchar_t can be up to cvt.max_length() chars, so we make our
				// buffer len * max to be able to fit it all.
				// The vector is used as a buffer to hold the string. While the &* trick
				// is obscure, it is believed to be correct, and it is also exception safe.
				std::size_t maxlen = from.length() * cvt.max_length();
				std::vector<char> arr(maxlen, L' ');

				// Setting up the state information needed by the codecvt facet.
				std::mbstate_t state = std::mbstate_t();
				const wchar_t *fromnext = NULL;
				char *tonext = NULL;
				codecvt::result r = cvt.out(state,
				                            from.data(), from.data() + from.length(), fromnext,
				                            &*arr.begin(), &*arr.end(), tonext);
							    
				// This could probably be made smarter, but it's hard to say what the smart thing to
				// do is given a partial or error response. Given use cases, a more robust interface could
				// probably be derived.
				if (r != codecvt::ok)
					throw std::bad_cast();
					
				return std::string(&*arr.begin(), tonext);
			}
		};
# endif
	}

	// just pass an equal string type straight through
	inline const tstring &convert(
		const tstring &from, const std::locale &loc = std::locale()) throw()
	{
		return from;
	}

	// all of the following conversions, including the explicitly templated conversions
	// are not possible without unicode, so they go away if the user says to get rid of
	// them. Obviously, code that expects them will not work without _NOWCHAR.
# if !defined(_NOWCHAR)
	// otherwise we do a real conversion.
	inline tstring convert(
        const convert_impl::opposite_type<tstring>::type &from, const std::locale &loc = std::locale())
	{
		return convert_impl::converter<tstring::value_type>::convert(from, loc);
	}

	// templated version for converting explicitly
	// call as convert<char>(L"blah"); or convert<wchar_t>("blah");
	template <typename tTo>
	inline const std::basic_string<tTo> &convert(const std::basic_string<tTo> &from, const std::locale &loc = std::locale()) throw()
	{
		return from;
	}

	template <typename tTo>
	inline std::basic_string<tTo> convert(
        const typename convert_impl::opposite_type< std::basic_string<tTo> >::type &from, const std::locale &loc = std::locale())
	{
		return convert_impl::converter<tTo>::convert(from, loc);
	}
# endif
}

#endif
