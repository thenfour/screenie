/*
  LibCC Release "March 9, 2007"
  StringUtil Module
  (c) 2004-2007 Carl Corcoran, carlco@gmail.com
  Documentation: http://wiki.winprog.org/wiki/LibCC

	== License:

  All software on this site is provided 'as-is', without any express or
  implied warranty, by its respective authors and owners. In no event will
  the authors be held liable for any damages arising from the use of this
  software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
  claim that you wrote the original software. If you use this software in
  a product, an acknowledgment in the product documentation would be
  appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not
  be misrepresented as being the original software.

  3. This notice may not be removed or altered from any source distribution.
*/

#pragma once

#include <string>
#include <tchar.h>
#include <malloc.h>// for alloca()
#include <math.h>// for fmod()
#include <locale>

#include "Blob.h"

#ifdef WIN32
# include <windows.h>// for GetLastError() / LoadStrin / FormatMessage...
#endif


#pragma warning(push)

/*
  Disable "conditional expression is constant".  This code has conditions
  on integral template params, which are by design constant.  This is a lame
  warning.
*/
#pragma warning(disable:4127)
#pragma warning(disable:4312)// CharUpper() and CharLower() require me to cast some char -> PWSTR, which gives this error.
#pragma warning(disable:4311)// CharUpper() and CharLower() require me to cast some char -> PWSTR, which gives this error.


#define CCSTR_OPTION_AUTOCAST 0// set this to 1 and class Format can auto-cast into std::string

// Set up inline option
#ifdef _MSC_VER
# if (LIBCC_OPTION_INLINE == 1)// set this option
#   define LIBCC_INLINE __declspec(noinline)
# else
#   define LIBCC_INLINE inline
# endif
#else
# define LIBCC_INLINE inline
#endif

namespace LibCC
{
  // this simple class just "attaches" to a float and provides a window into it's inner workings.
  // based on information from http://www.duke.edu/~twf/cps104/floating.html
  // ... and http://stevehollasch.com/cgindex/coding/ieeefloat.html
  // e notation:    d.dd...En
  // 1 bit = sign
  // 8 bits = exp
  // 23 bits = mantissa
  /*
    What are denormalized numbers?  they are normal numbers but without the leading 1 and assumed exp of -126
  */
  template<typename _BasicType,
    typename _InternalType,
    typename _Exponent,
    typename _Mantissa,
    long _ExponentBits,
    long _MantissaBits>
  class IEEEFloat
  {
  public:
    typedef _BasicType BasicType;
    typedef _InternalType InternalType;
    typedef _Exponent Exponent;
    typedef _Mantissa Mantissa;
    typedef IEEEFloat<BasicType, InternalType, Exponent, Mantissa, _ExponentBits, _MantissaBits> This;

    static const long ExponentBits = _ExponentBits;// 0x08 / 0x0b
    static const long MantissaBits = _MantissaBits;// 0x17 / 0x34
    static const _InternalType SignMask = (InternalType)1 << (ExponentBits + MantissaBits);// 0x80000000 / 0x8000000000000000
    static const _InternalType MantissaMask = ((InternalType)1 << MantissaBits) - 1;// 0x007fffff / 0x000fffffffffffff
    static const _InternalType MantissaHighBit = MantissaMask ^ (MantissaMask >> 1);// 0x00400000 / 0x0008000000000000
    static const _InternalType ExponentMask = ((((InternalType)1 << ExponentBits) - 1) << MantissaBits);// 0x7f800000 / 0x7ff0000000000000
    static const _InternalType PositiveInfinity = ExponentMask;// 0x7f800000 / 0x7ff0000000000000
    static const _InternalType NegativeInfinity = SignMask | ExponentMask;// 0xff800000 / 0xfff0000000000000
    static const Exponent ExponentBias = (((InternalType)1 << (ExponentBits-1)) - 1);// 0x7f / 0x3ff

    IEEEFloat(BasicType f) :
      m_val(reinterpret_cast<InternalType&>(m_BasicVal)),
      m_BasicVal(f)
    {
    }

    IEEEFloat(const This& r) :
      m_val(reinterpret_cast<InternalType&>(m_BasicVal)),
      m_BasicVal(r.m_BasicVal)
    {
    }

    IEEEFloat(_InternalType r) :
      m_val(reinterpret_cast<InternalType&>(m_BasicVal))
    {
      m_val = r;
    }

    bool IsPositive() const
    {
      return m_val & SignMask ? false : true;
    }
    bool IsNegative() const
    {
      return !IsPositive();
    }
    bool IsZero() const// exponent == 0  &&  mantissa == 0
    {
      return m_val & (MantissaMask | ExponentMask) ? false : true;
    }
    bool IsDenormalized() const// exponent = 0  &&  mantissa != 0
    {
      return (!(m_val & ExponentMask)) && (m_val & MantissaMask));
    }
    bool IsPositiveInfinity() const// exponent == MAX
    {
      return m_val == PositiveInfinity;
    }
    bool IsNegativeInfinity() const// exponent == MAX
    {
      return m_val == NegativeInfinity;
    }
    bool IsInfinity() const// exponent == MAX  &&  mantissa == MAX
    {
      return (m_val & (MantissaMask | ExponentMask)) == (MantissaMask | ExponentMask);
    }
    bool IsNaN() const// exponent == MAX  && mantissa != 0
    {
      return ((m_val & ExponentMask) == ExponentMask) && (m_val & MantissaMask);
    }
    bool IsQNaN() const// exponent == MAX  && mantissa high bit set
    {
      return ((m_val & ExponentMask) == ExponentMask) && (m_val & MantissaHighBit);
    }
    bool IsSNaN() const// exponent == MAX  && mantissa != 0  && mantissa high bit 0
    {
      return ((m_val & ExponentMask) == ExponentMask) && (m_val & MantissaMask) && !(m_val & MantissaHighBit);
    }
    Exponent GetExponent() const
    {
      return static_cast<Exponent>(((m_val & ExponentMask) >> MantissaBits) - ExponentBias);
    }

    Mantissa GetMantissa() const
    {
      return static_cast<Mantissa>(m_val & MantissaMask | (static_cast<Mantissa>(1) << MantissaBits));// add the implied 1
    }

    void CopyValue(BasicType& out) const
    {
      memcpy(&out, &m_val, sizeof(m_val));
    }

    static This Build(bool Sign, Exponent ex, Mantissa m)
    {
      InternalType r;
      r = Sign ? SignMask : 0;// sign
      r |= (ex + ExponentShift) << MantissaBits;// exponent
      r |= m & MantissaMask;// mantissa
      return *(reinterpret_cast<BasicType*>(&r));
    }

    /*
      Infinity
      The values +infinity and -infinity are denoted with an exponent of all 1s and a
      fraction of all 0s. The sign bit distinguishes between negative infinity and positive
      infinity.
    */
    static This BuildPositiveInfinity()
    {
      InternalType r;
      r = ExponentMask;
      return *(reinterpret_cast<BasicType*>(&r));
    }

    static This BuildNegativeInfinity()
    {
      InternalType r;
      r = SignMask | ExponentMask;
      return *(reinterpret_cast<BasicType*>(&r));
    }

    // The value NaN (Not a Number) is used to represent a value that does not represent a real number.
    // NaN's are represented by a bit pattern with an exponent of all 1s and a non-zero fraction.
    // There are two categories of NaN: QNaN (Quiet NaN) and SNaN (Signalling NaN).
    // A QNaN is a NaN with the most significant fraction bit set.
    // QNaN's propagate freely through most arithmetic operations. These values pop out of an operation when the result is not mathematically defined.
    static This BuildQNaN()
    {
      InternalType r;
      r = ExponentMask | MantissaMask;
      return *(reinterpret_cast<BasicType*>(&r));
    }

    // An SNaN is a NaN with the most significant fraction bit clear. It is used to signal an exception when used in operations. SNaN's can be handy to assign to uninitialized variables to trap premature usage. 
    static This BuildSNaN()
    {
      InternalType r;
      r = ExponentMask | (MantissaMask >> 1);
      return *(reinterpret_cast<BasicType*>(&r));
    }

    void AbsoluteValue()
    {
      m_val &= ~SignMask;
    }

    /*
      say you have a mantissa:
      1.0101101 with exponent 3.
      that's: 1010.1101
      to remove the decimal, it would be:
      that's: 1010.0
      or, 1.01, exponent 3
      In the case of a negative exponent, set the float to zero.

      So the idea is simply to mask out the bits that are right of
      the decimal point.
    */
    void RemoveDecimal()
    {
      InternalType e = (m_val & ExponentMask) >> MantissaBits;
      Mantissa m = static_cast<Mantissa>(m_val & MantissaMask);

      if(m)
      {
        if(e >= ExponentBias)
        {
          e -= ExponentBias;
          if(e < MantissaBits)// if we did this when e is greater than the mantissa bits, it would get all screwy.
          {
            // positive exponent.  mask out the decimal part.
            InternalType mask = 1;
            mask <<= (MantissaBits - e);// bits
            mask -= 1;// mask
            m_val &= ~mask;
          }
        }
        else
        {
          // negative exponent; set this float to zero, retaining the current sign.
          m_val &= ~(ExponentMask | MantissaMask);
        }
      }
    }

    InternalType& m_val;
    BasicType m_BasicVal;

    private:
      This& operator =(const This& rhs) { return *this; }// do not allow assignment.  this is also to prevent warning C4512 "'class' : assignment operator could not be generated"

  };
  typedef IEEEFloat<float, unsigned __int32, signed __int8, unsigned __int32, 8, 23> SinglePrecisionFloat;
  typedef IEEEFloat<double, unsigned __int64, signed __int16, unsigned __int64, 11, 52> DoublePrecisionFloat;
}























// string utility functions































namespace LibCC
{
  inline char DigitToChar(unsigned char d)
  {
    static const char Digits [] = "0123456789abcdefghijklmnopqrstuvwxyz";
    return d < (sizeof(Digits) / sizeof(char)) ? Digits[d] : 0;
  }

	// other things to consider: quotes around tokens, escape characters
  template<typename Char, class OutIt>
  inline void StringSplit(const std::basic_string<Char>& s, const std::basic_string<Char>& sep, OutIt dest)
  {
		if(s.empty())
		{
			return;
		}
		if(sep.empty())
		{
			*dest = s;
			++ dest;
			return;
		}
		typedef std::basic_string<Char> _String;
		_String token;
		_String::const_iterator sepIt = sep.begin();
		
		for(_String::const_iterator it = s.begin(); it != s.end(); ++ it)
		{
			if(*it == *sepIt)
			{
				sepIt ++;
				if(sepIt == sep.end())
				{
					// we found the whole separator. push the previous token.
					*dest = token;
					++ dest;
					token.clear();
					sepIt = sep.begin();
					// and if we are at the very end of the string, add a blank token (like "a,b,c," will give 4 results = a, b, c, and a blank one)
					if(it + 1 == s.end())
					{
						*dest = token;
						++ dest;
						break;
					}
				}
				else
				{
					// just another character in the separator.
				}
			}
			else
			{
				// just another character in a TOKEN
				token.push_back(*it);
				sepIt = sep.begin();
			}
		}
		// if there's a token at the end of the string, append it. blank tokens would have already been added.
		if(!token.empty())
		{
			*dest = token;
			++ dest;
		}
		return;
  }
  template<typename Char, class OutIt>
  inline void StringSplit(const std::basic_string<Char>& s, const Char* sep, OutIt dest)
  {
		StringSplit(s, std::basic_string<Char>(sep), dest);
  }
  template<typename Char, class OutIt>
  inline void StringSplit(const Char* s, const std::basic_string<Char>& sep, OutIt dest)
  {
		StringSplit(std::basic_string<Char>(s), sep, dest);
  }
  template<typename Char, class OutIt>
  inline void StringSplit(const Char* s, const Char* sep, OutIt dest)
  {
		StringSplit(std::basic_string<Char>(s), std::basic_string<Char>(sep), dest);
  }

  template<typename InIt, typename Char>
  inline std::basic_string<Char> StringJoin(InIt start, InIt end, const std::basic_string<Char>& sep)
  {
    std::basic_string<Char> r;
    while(start != end)
    {
      r.append(*start);
      ++ start;
      if(start != end)
      {
        r.append(sep);
      }
    }
    return r;
  }
  template<typename InIt, typename Char>
  inline std::basic_string<Char> StringJoin(InIt start, InIt end, const Char* sep)
  {
		return StringJoin(start, end, std::basic_string<Char>(sep));
  }
  
  template<typename Char>
  inline std::basic_string<Char> StringTrim(const std::basic_string<Char>& s, const std::basic_string<Char>& chars)
  {
    std::basic_string<Char>::size_type left = s.find_first_not_of(chars);
    std::basic_string<Char>::size_type right = s.find_last_not_of(chars);
    if((right == std::basic_string<Char>::npos) || (left == std::basic_string<Char>::npos))
    {
      return std::basic_string<Char>();
    }
    return std::basic_string<Char>(s, left, 1 + right - left);
  }
  template<typename Char>
  inline std::basic_string<Char> StringTrim(const std::basic_string<Char>& s, const Char* chars)
  {
		return StringTrim(s, std::basic_string<Char>(chars));
  }
  template<typename Char>
  inline std::basic_string<Char> StringTrim(const Char* s, const std::basic_string<Char>& chars)
  {
		return StringTrim(std::basic_string<Char>(s), chars);
  }
  template<typename Char>
  inline std::basic_string<Char> StringTrim(const Char* s, const Char* chars)
  {
		return StringTrim(std::basic_string<Char>(s), std::basic_string<Char>(chars));
  }

  template<typename Char>
  inline std::basic_string<Char> StringReplace(const std::basic_string<Char>& src, const std::basic_string<Char>& searchString, const std::basic_string<Char>& replaceString)
  {
		if(searchString.empty()) return src;// you can't have a 0 length search string.
    typedef std::basic_string<Char> _String;
    _String r;
    size_t pos = 0;
		while(1)
		{
			size_t found = src.find(searchString, pos);
			if(found == _String::npos)
			{
				// append the remainder of src.
				r.append(src, pos, src.length() - pos);
				return r;
			}
			r.append(src, pos, found - pos);// append chunk from src.
			r.append(replaceString);// append replacement
			pos = found + searchString.length();// advance
		}
  }
  template<typename Char>// these overloads help compile with constant strings
  inline std::basic_string<Char> StringReplace(const std::basic_string<Char>& src, const std::basic_string<Char>& searchString, const Char* replaceString)
  {
		return StringReplace(src, searchString, std::basic_string<Char>(replaceString));
	}
  template<typename Char>
  inline std::basic_string<Char> StringReplace(const std::basic_string<Char>& src, const Char* searchString, const std::basic_string<Char>& replaceString)
  {
		return StringReplace(src, std::basic_string<Char>(searchString), replaceString);
	}
  template<typename Char>
  inline std::basic_string<Char> StringReplace(const std::basic_string<Char>& src, const Char* searchString, const Char* replaceString)
  {
		return StringReplace(src, std::basic_string<Char>(searchString), std::basic_string<Char>(replaceString));
	}
  template<typename Char>
  inline std::basic_string<Char> StringReplace(const Char* src, const std::basic_string<Char>& searchString, const std::basic_string<Char>& replaceString)
  {
		return StringReplace(std::basic_string<Char>(src), searchString, replaceString);
	}
  template<typename Char>
  inline std::basic_string<Char> StringReplace(const Char* src, const std::basic_string<Char>& searchString, const Char* replaceString)
  {
		return StringReplace(std::basic_string<Char>(src), searchString, std::basic_string<Char>(replaceString));
	}
  template<typename Char>
  inline std::basic_string<Char> StringReplace(const Char* src, const Char* searchString, const std::basic_string<Char>& replaceString)
  {
		return StringReplace(std::basic_string<Char>(src), std::basic_string<Char>(searchString), replaceString);
	}
  template<typename Char>
  inline std::basic_string<Char> StringReplace(const Char* src, const Char* searchString, const Char* replaceString)
  {
		return StringReplace(std::basic_string<Char>(src), std::basic_string<Char>(searchString), std::basic_string<Char>(replaceString));
	}

	// NOTE: the stdlib toupper functions do not handle unicode very well, so this will have to do.
	inline std::wstring StringToUpper(const std::wstring& s)
	{
		Blob<wchar_t> buf(s.length() + 1);
		wcscpy(buf.GetBuffer(), s.c_str());
		CharUpperBuffW(buf.GetBuffer(), (DWORD)s.length());
		return std::wstring(buf.GetBuffer());
	}
	inline std::string StringToUpper(const std::string& s)
	{
		Blob<char> buf(s.length() + 1);
		strcpy(buf.GetBuffer(), s.c_str());
		CharUpperBuffA(buf.GetBuffer(), (DWORD)s.length());
		return std::string(buf.GetBuffer());
	}
	template<typename Char>
	inline std::basic_string<Char> StringToUpper(const std::basic_string<Char>& s)// last-ditch for alternative char types
	{
    std::basic_string<Char> r;
    r.reserve(s.size());
    std::basic_string<Char>::const_iterator it;
    for(it = s.begin(); it != s.end(); ++ it)
    {
			if(*it < 0x8000)
			{
				r.push_back((Char)CharUpperW((PWSTR)*it));
      }
      else
      {
				r.push_back(*it);
      }
    }
    return r;
	}
  template<typename Char>
  inline std::basic_string<Char> StringToUpper(const Char* s)
  {
		return StringToUpper(std::basic_string<Char>(s));
  }


	inline std::wstring StringToLower(const std::wstring& s)
	{
		Blob<wchar_t> buf(s.length() + 1);
		wcscpy(buf.GetBuffer(), s.c_str());
		CharLowerBuffW(buf.GetBuffer(), (DWORD)s.length());
		return std::wstring(buf.GetBuffer());
	}
	inline std::string StringToLower(const std::string& s)
	{
		Blob<char> buf(s.length() + 1);
		strcpy(buf.GetBuffer(), s.c_str());
		CharLowerBuffA(buf.GetBuffer(), (DWORD)s.length());
		return std::string(buf.GetBuffer());
	}
	template<typename Char>
	inline std::basic_string<Char> StringToLower(const std::basic_string<Char>& s)// last-ditch for alternative char types
	{
    std::basic_string<Char> r;
    r.reserve(s.size());
    std::basic_string<Char>::const_iterator it;
    for(it = s.begin(); it != s.end(); ++ it)
    {
			if(*it < 0x8000)
			{
				r.push_back((Char)CharLowerW((PWSTR)*it));
      }
      else
      {
				r.push_back(*it);
      }
    }
    return r;
	}
  template<typename Char>
  inline std::basic_string<Char> StringToLower(const Char* s)
  {
		return StringToLower(std::basic_string<Char>(s));
  }


  template<typename Char, typename Trhs>
  inline bool StringEquals(const std::basic_string<Char>& lhs, const Trhs& rhs)
  {
		return lhs == rhs;
  }
  template<typename Char, typename Trhs>
  inline bool StringEquals(const Char* lhs, const Trhs& rhs)
  {
		return std::basic_string<Char>(lhs) == rhs;
  }

	/*
		Convenience functions for other LibCC code like the registry stuff that has a Char template parameter,
		but needs to hard-code string constants, and interact between the two. The hard-coded strings will all
		be "typical" ASCII strings, and so we don't need to worry about problems of MBCS here.
	*/
  template<typename Char>
	inline bool XStringEquals(const std::basic_string<Char>& lhs, const char* rhs)
	{
		if(lhs.size() > strlen(rhs)) return false;
		for(std::basic_string<Char>::const_iterator it = lhs.begin(); it != lhs.end(); ++ it, ++ rhs)
		{
			if(*it != *rhs) return false;
		}
		return *rhs == 0;
	}
	
  template<typename Char>
  inline bool XStringContains(const char* source, Char x)
  {
    while(*source)
    {
      if(*source == x) return true;
      source ++;
    }
    return false;
  }
  
  template<typename Char>
  inline std::string::size_type XStringFindFirstOf(const std::basic_string<Char>& s, const char* chars)
  {
    std::string::size_type ret = 0;
    for(std::basic_string<Char>::const_iterator it = s.begin(); it != s.end(); ++ it)
    {
      if(XStringContains(chars, *it))
      {
        return ret;
      }
      ret ++;
    }
    return std::string::npos;
  }
  
  template<typename Char>
  inline std::string::size_type XStringFindLastOf(const std::basic_string<Char>& s, const char* chars)
  {
    std::string::size_type ret = 0;
    for(std::basic_string<Char>::const_reverse_iterator it = s.rbegin(); it != s.rend(); ++ it)
    {
      if(XStringContains(chars, *it))
      {
        return s.size() - ret - 1;
      }
      ret ++;
    }
    return std::string::npos;
  }
  template<typename Char>
	inline size_t XStringLength(const Char* sz)
	{
		size_t ret = 0;
		while(*sz ++)
		{
			++ ret;
		}
		return ret;
	}
  template<typename InChar, typename OutChar>
	inline void XLastDitchStringCopy(const std::basic_string<InChar>& in, std::basic_string<OutChar>& out)
	{
		// when there's no other way to convert from 1 string type to another, you can try this basic element-by-element copy
		out.clear();
		out.reserve(in.size());
		for(std::basic_string<InChar>::const_iterator it = in.begin(); it != in.end(); ++ it)
		{
			out.push_back(static_cast<OutChar>(*it));
		}
	}
  template<typename InChar, typename OutChar>
	inline void XLastDitchStringCopy(const InChar* in, std::basic_string<OutChar>& out)
	{
		// when there's no other way to convert from 1 string type to another, you can try this basic element-by-element copy
		out.clear();
		out.reserve(XStringLength(in));
		for(; *in != 0; ++ in)
		{
			out.push_back(static_cast<OutChar>(*in));
		}
	}
  template<typename InChar, typename OutChar>
	inline void XLastDitchStringCopy(const InChar* in, OutChar* out)// out must already be allocated
	{
		for(; *in != 0; ++ in)
		{
			*out = *in;
			out ++;
		}
		*out = 0;
	}
  template<typename InChar, typename OutChar>
	inline void XLastDitchStringCopy(const std::basic_string<InChar>& in, OutChar* out)// out must already be allocated
	{
		for(std::basic_string<InChar>::const_iterator it = in.begin(); it != in.end(); ++ it)
		{
			*out = *it;
			out ++;
		}
		*out = 0;
	}


#ifdef WIN32
	inline HRESULT ConvertString(const std::wstring& widestr, Blob<BYTE>& out, UINT codepage = CP_ACP)
	{
		DWORD flags;
		CPINFO cpinfo;
	 
		switch (codepage)
		{
		case 50220: case 50221: case 50222: case 50225:
		case 50227: case 50229: case 52936: case 54936:
		case 57002: case 57003: case 57004: case 57005:
		case 57006: case 57007: case 57008: case 57009:
		case 57010: case 57011: case 65000: case 42:
			flags = 0;
			break;
		case 65001:
			flags = 0; // or WC_ERR_INVALID_CHARS on winver >= 0x0600
			break;
		default:
			{
				flags = WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK;
				break;
			}
		}

		if(0 == GetCPInfo(codepage, &cpinfo))
		{
			return E_FAIL;
		}
	 
		// get the length first
		BOOL usedDefaultChar = FALSE;
		int length = WideCharToMultiByte(codepage, flags, widestr.c_str(), static_cast<int>(widestr.length()), NULL, 0,
			(flags & WC_NO_BEST_FIT_CHARS) ? (const CHAR*)cpinfo.DefaultChar : 0,
			(flags & WC_NO_BEST_FIT_CHARS) ? &usedDefaultChar : 0);

		if (length == 0)
			return E_FAIL;

		out.Alloc(length);// it is important to make sure the return Blob has the correct size here. so do not add +1 to this.

		WideCharToMultiByte(codepage, flags, widestr.c_str(), (int)widestr.length(), (LPSTR)out.GetBuffer(), length,
			(flags & WC_NO_BEST_FIT_CHARS) ? (const CHAR*)cpinfo.DefaultChar : 0,
			(flags & WC_NO_BEST_FIT_CHARS) ? &usedDefaultChar : 0);

		return S_OK;
	}
	inline HRESULT ConvertString(const std::wstring& widestr, std::string& out, UINT codepage = CP_ACP)
	{
		Blob<BYTE> b;
		HRESULT hr = ConvertString(widestr, b, codepage);
		if(FAILED(hr)) return hr;
		out.assign((const char*)b.GetBuffer(), b.Size());
		return hr;
	}
	template<typename Char>
	inline HRESULT ConvertString(const std::basic_string<Char>& lhs, std::string& out)
	{
		XLastDitchStringCopy(lhs, out);
		return S_OK;
	}
	template<typename Char>
	inline HRESULT ConvertString(const Char* lhs, std::string& out)
	{
		XLastDitchStringCopy(lhs, out);
		return S_OK;
	}
	inline HRESULT ConvertString(const BYTE* multistr, size_t sourceLength, std::wstring& widestr, UINT codepage = CP_ACP)
	{
		int length = MultiByteToWideChar(codepage, 0, (PCSTR)multistr, (int)sourceLength, NULL, 0);
		if (length == 0)
			return E_FAIL;
		Blob<WCHAR> buf;
		if(!buf.Alloc(length))
		{
			return E_OUTOFMEMORY;
		}
		MultiByteToWideChar(codepage, 0, (PCSTR)multistr, (int)sourceLength, buf.GetBuffer(), (int)length);
		widestr.assign(buf.GetBuffer(), buf.Size());
 
		return S_OK;
	}

	inline HRESULT ConvertString(const Blob<BYTE>& multistr, std::wstring& widestr, UINT codepage = CP_ACP)
	{
		return ConvertString(multistr.GetBuffer(), multistr.Size(), widestr, codepage);
	}

	inline HRESULT ConvertString(const char* multistr, std::wstring& widestr, UINT codepage = CP_ACP)
	{
		return ConvertString((const BYTE*)multistr, strlen(multistr), widestr, codepage);
	}

	inline HRESULT ConvertString(const std::string& multistr, std::wstring& widestr, UINT codepage = CP_ACP)
	{
		return ConvertString((const BYTE*)multistr.c_str(), multistr.length(), widestr, codepage);
	}

	inline HRESULT ConvertString(const std::wstring& lhs, std::wstring& widestr, UINT codepage = CP_ACP)// codepage is technically ignored here. again this is just a convenience function to aide compilation.
	{
		widestr = lhs;
		return S_OK;
	}
	inline HRESULT ConvertString(const wchar_t* lhs, std::wstring& widestr, UINT codepage = CP_ACP)// codepage is technically ignored here. again this is just a convenience function to aide compilation.
	{
		widestr = lhs;
		return S_OK;
	}
	template<typename CharA, typename CharB>
	inline HRESULT ConvertString(const std::basic_string<CharA>& lhs, std::basic_string<CharB>& widestr, UINT codepage = CP_ACP)// codepage is technically ignored here. again this is just a convenience function to aide compilation.
	{
		XLastDitchStringCopy(lhs, widestr);
		return S_OK;
	}
	template<typename CharA, typename CharB>
	inline HRESULT ConvertString(const CharA* lhs, std::basic_string<CharB>& widestr, UINT codepage = CP_ACP)// codepage is technically ignored here. again this is just a convenience function to aide compilation.
	{
		XLastDitchStringCopy(lhs, widestr);
		return S_OK;
	}

	inline HRESULT ConvertString(const std::string& in, std::string& out, UINT fromCodepage = CP_ACP, UINT toCodepage = CP_ACP)// from 'char' string to 'char' string, the codepage is ignored.
	{
		if(fromCodepage == toCodepage)
		{
			out = in;
			return S_OK;
		}
		std::wstring intermediate;
		HRESULT hr = ConvertString(in, intermediate, fromCodepage);
		if(FAILED(hr)) return hr;
		return ConvertString(intermediate, out, toCodepage);
	}

	inline HRESULT ConvertString(const char* in, std::string& out, UINT fromCodepage = CP_ACP, UINT toCodepage = CP_ACP)
	{
		if(fromCodepage == toCodepage)
		{
			out = in;
			return S_OK;
		}
		std::wstring intermediate;
		HRESULT hr = ConvertString(in, intermediate, fromCodepage);
		if(FAILED(hr)) return hr;
		return ConvertString(intermediate, out, toCodepage);
	}

	inline HRESULT ConvertString(const std::string& in, Blob<BYTE>& out, UINT fromCodepage = CP_ACP, UINT toCodepage = CP_ACP)
	{
		if(fromCodepage == toCodepage)
		{
			if(!out.Alloc(in.length())) return E_OUTOFMEMORY;
			memcpy(out.GetBuffer(), in.c_str(), in.length());
			return S_OK;
		}
		std::wstring intermediate;
		HRESULT hr = ConvertString(in, intermediate, fromCodepage);
		if(FAILED(hr)) return hr;
		return ConvertString(intermediate, out, toCodepage);
	}

	inline HRESULT ConvertString(const char* in, Blob<BYTE>& out, UINT fromCodepage = CP_ACP, UINT toCodepage = CP_ACP)
	{
		if(fromCodepage == toCodepage)
		{
			size_t len = XStringLength(in);
			if(!out.Alloc(len)) return E_OUTOFMEMORY;
			memcpy(out.GetBuffer(), in, len);
			return S_OK;
		}
		std::wstring intermediate;
		HRESULT hr = ConvertString(in, intermediate, fromCodepage);
		if(FAILED(hr)) return hr;
		return ConvertString(intermediate, out, toCodepage);
	}

	inline HRESULT ConvertString(const BYTE* in, size_t inLength, Blob<BYTE>& out, UINT fromCodepage = CP_ACP, UINT toCodepage = CP_ACP)
	{
		if(fromCodepage == toCodepage)
		{
			if(!out.Alloc(inLength)) return E_OUTOFMEMORY;
			memcpy(out.GetBuffer(), in, inLength);
			return S_OK;
		}
		std::wstring intermediate;
		HRESULT hr = ConvertString(in, inLength, intermediate, fromCodepage);
		if(FAILED(hr)) return hr;
		return ConvertString(intermediate, out, toCodepage);
	}

	inline HRESULT ConvertString(const Blob<BYTE>& in, Blob<BYTE>& out, UINT fromCodepage = CP_ACP, UINT toCodepage = CP_ACP)
	{
		return ConvertString(in.GetBuffer(), in.Size(), out, fromCodepage, toCodepage);
	}

	inline HRESULT ConvertString(const Blob<BYTE>& in, std::string& out, UINT fromCodepage = CP_ACP, UINT toCodepage = CP_ACP)
	{
		if(fromCodepage == toCodepage)
		{
			out.assign((const char*)in.GetBuffer(), in.Size());
			return S_OK;
		}
		std::wstring intermediate;
		HRESULT hr = ConvertString(in, intermediate, fromCodepage);
		if(FAILED(hr)) return hr;
		return ConvertString(intermediate, out, toCodepage);
	}

	template<typename Char>
	inline HRESULT ConvertString(const std::wstring& in, std::basic_string<Char>& out)
	{
		XLastDitchStringCopy(in, out);
		return S_OK;
	}

	template<typename Char>
	inline HRESULT ConvertString(const Blob<BYTE>& in, std::basic_string<Char>& out, UINT fromCodepage = CP_ACP)
	{
		// convert to unicode, then to Blob
		std::wstring intermediate;
		HRESULT hr = ConvertString(in, intermediate, fromCodepage);
		if(FAILED(hr)) return hr;
		return ConvertString(intermediate, out);
	}

	template<typename Char>
	inline HRESULT ConvertString(const Char* in, Blob<BYTE>& out, UINT toCodepage = CP_ACP)
	{
		// convert to unicode, then to Blob
		std::wstring intermediate;
		HRESULT hr = ConvertString(in, intermediate);
		if(FAILED(hr)) return hr;
		return ConvertString(intermediate, out, toCodepage);
	}

	template<typename Char>
	inline HRESULT ConvertString(const std::basic_string<Char>& in, Blob<BYTE>& out, UINT toCodepage = CP_ACP)
	{
		// convert to unicode, then to Blob
		std::wstring intermediate;
		HRESULT hr = ConvertString(in, intermediate);
		if(FAILED(hr)) return hr;
		return ConvertString(intermediate, out, toCodepage);
	}

	inline HRESULT ConvertString(const BYTE* in, size_t inLength, std::string& out, UINT fromCodepage = CP_ACP, UINT toCodepage = CP_ACP)
	{
		if(fromCodepage == toCodepage)
		{
			out.assign((const char*)in, inLength);
			return S_OK;
		}
		std::wstring intermediate;
		HRESULT hr = ConvertString(in, inLength, intermediate, fromCodepage);
		if(FAILED(hr)) return hr;
		return ConvertString(intermediate, out, toCodepage);
	}

	template<typename Char>
	inline HRESULT ConvertString(const BYTE* in, size_t inLength, std::basic_string<Char>& out, UINT fromCodepage = CP_ACP)
	{
		// convert to unicode, then to Blob
		std::wstring intermediate;
		HRESULT hr = ConvertString(in, inLength, intermediate, fromCodepage);
		if(FAILED(hr)) return hr;
		return ConvertString(intermediate, out);
	}
	
	// ToUnicode .... jeez. i'm lucky there's no A/W version of this winapi or i'd have to find a new name.
	template<typename Char>
	inline std::wstring ToUnicode(const Char* sz, UINT codepage = CP_ACP)
	{
		std::wstring ret;
		ConvertString(sz, ret, codepage);
		return ret;
	}
	template<typename Char>
	inline std::wstring ToUnicode(const std::basic_string<Char>& s, UINT codepage = CP_ACP)
	{
		std::wstring ret;
		ConvertString(s, ret, codepage);
		return ret;
	}
	inline std::wstring ToUnicode(const BYTE* mbcs, size_t len, UINT codepage = CP_ACP)
	{
		std::wstring ret;
		ConvertString(mbcs, len, ret, codepage);
		return ret;
	}
	inline std::wstring ToUnicode(const Blob<BYTE> mbcs, UINT codepage = CP_ACP)
	{
		std::wstring ret;
		ConvertString(mbcs, ret, codepage);
		return ret;
	}
	
	// ToMBCS.... jeez. i'm lucky there's no A/W version of this winapi or i'd have to find a new name.
	template<typename Char>
	inline std::string ToMBCS(const Char* sz, UINT codepage = CP_ACP)
	{
		std::string ret;
		ConvertString(sz, ret, codepage);
		return ret;
	}
	template<typename Char>
	inline std::string ToMBCS(const std::basic_string<Char>& s, UINT codepage = CP_ACP)
	{
		std::string ret;
		ConvertString(s, ret, codepage);
		return ret;
	}
	inline std::string ToMBCS(const BYTE* mbcs, size_t len, UINT codepage = CP_ACP)
	{
		std::string ret;
		ConvertString(mbcs, len, ret, codepage);
		return ret;
	}
	inline std::string ToMBCS(const Blob<BYTE> mbcs, UINT codepage = CP_ACP)
	{
		std::string ret;
		ConvertString(mbcs, ret, codepage);
		return ret;
	}

	// ToUTF8
	inline HRESULT ToUTF8(const std::wstring& widestr, std::string& out)
	{
		return ConvertString(widestr, out, CP_UTF8);
	}
	inline HRESULT ToUTF8(const wchar_t* widestr, std::string& out)
	{
		return ConvertString(widestr, out, CP_UTF8);
	}
	inline HRESULT ToUTF8(const char* in, std::string& out)
	{
		std::wstring intermediate;
		HRESULT hr = ConvertString(in, intermediate, CP_UTF8);
		if(FAILED(hr)) return hr;
		return ConvertString(intermediate, out, CP_UTF8);
	}
	inline HRESULT ToUTF8(const std::string& in, std::string& out)
	{
		std::wstring intermediate;
		HRESULT hr = ConvertString(in, intermediate, CP_UTF8);
		if(FAILED(hr)) return hr;
		return ConvertString(intermediate, out, CP_UTF8);
	}

	template<typename Char>
	inline std::string ToUTF8(const Char* sz)
	{
		return ToMBCS(sz, CP_UTF8);
	}
	template<typename Char>
	inline std::string ToUTF8(const std::basic_string<Char>& s)
	{
		return ToMBCS(s, CP_UTF8);
	}
	inline std::string ToUTF8(const BYTE* mbcs, size_t len)
	{
		return ToMBCS(mbcs, len, CP_UTF8);
	}
	inline std::string ToUTF8(const Blob<BYTE> mbcs)
	{
		return ToMBCS(mbcs, CP_UTF8);
	}



#endif

}






























// LibCC::Format coming up...




























namespace LibCC
{
  // FormatX class declaration -----------------------------------------------------------------------------------
  template<typename Ch = char, typename Traits = std::char_traits<Ch>, typename Alloc = std::allocator<Ch> >
  class FormatX
  {
  public:
    typedef Ch _Char;
    typedef Traits _Traits;
    typedef Alloc _Alloc;
    typedef std::basic_string<_Char, _Traits, _Alloc> _String;
    typedef FormatX<_Char, _Traits, _Alloc> _This;

		static const _Char OpenQuote = '\"';
		static const _Char CloseQuote = '\"';

    static const _Char ReplaceChar = '%';
    static const _Char EscapeChar = '^';
    static const _Char NewlineChar = '|';

		inline static bool IsUnicode()
		{
			return sizeof(_Char) == sizeof(wchar_t);
		}

		// notepad, winword, ultraedit do not support these characters
		// but wordpad & devenv do. not really enough support to justify using these ever, considering anything that supports them will also support \r\n
		inline static void AppendNewLine(_String& s)
		{
#if LIBCC_UNICODENEWLINES == 1
			if(IsUnicode())
			{
				s.push_back(0x2028);// the Unicode line separator char
			}
			else
			{
				s.push_back('\r');
				s.push_back('\n');
			}
#else
				s.push_back('\r');
				s.push_back('\n');
#endif
		}

		inline static void AppendNewParagraph(_String& s)
		{
#if LIBCC_UNICODENEWLINES == 1
			if(IsUnicode())
			{
				s.push_back(0x2029);// the Unicode new paragraph char
			}
			else
			{
				s.push_back('\r');
				s.push_back('\n');
			}
#else
				s.push_back('\r');
				s.push_back('\n');
#endif
		}

    // Construction / Assignment
		LIBCC_INLINE FormatX() :
			m_pos(0)
		{
		}

    LIBCC_INLINE FormatX(const _This& r) :
			m_pos(r.m_pos),
			m_Format(r.m_Format),
			m_Composite(r.m_Composite)
		{
		}

		explicit LIBCC_INLINE FormatX(const _String& s) :
			m_Format(s),
			m_pos(0)
		{
			m_Composite.reserve(m_Format.size());
			BuildCompositeChunk();
		}

    explicit LIBCC_INLINE FormatX(const _Char* s) :
			m_Format(s),
			m_pos(0)
		{
			if(!s) return;
			m_Composite.reserve(m_Format.size());
			BuildCompositeChunk();
		}

    template<typename CharX>
    explicit inline FormatX(const CharX* s) :
			m_pos(0)
		{
			if(!s) return;
			ConvertString(s, m_Format);
			BuildCompositeChunk();
		}

		template<typename CharX>
		explicit LIBCC_INLINE FormatX(const std::basic_string<CharX>& s) :
			m_pos(0)
		{
			ConvertString(s, m_Format);
			BuildCompositeChunk();
		}

#ifdef WIN32
    // construct from stringtable resource
    LIBCC_INLINE FormatX(HINSTANCE hModule, UINT stringID) :
			m_pos(0)
		{
			if(LoadStringX(hModule, stringID, m_Format))
			{
				m_Composite.reserve(m_Format.size());
				BuildCompositeChunk();
			}
		}

    LIBCC_INLINE FormatX(UINT stringID) :
			m_pos(0)
		{
			if(LoadStringX(GetModuleHandle(NULL), stringID, m_Format))
			{
				m_Composite.reserve(m_Format.size());
				BuildCompositeChunk();
			}
		}
#endif

		LIBCC_INLINE void Clear()
		{
			m_pos = 0;
			m_Format.clear();
			m_Composite.clear();
		}

    template<typename CharX>
    LIBCC_INLINE void SetFormat(const CharX* s)
		{
			if(!s)
			{
				Clear();
				return;
			}
			m_pos = 0;
			ConvertString(s, m_Format);
			m_Composite.clear();
			m_Composite.reserve(m_Format.size());
			BuildCompositeChunk();
		}

    LIBCC_INLINE void SetFormat(const _String& s)
		{
			m_pos = 0;
			m_Format = s;
			m_Composite.clear();
			m_Composite.reserve(m_Format.size());
			BuildCompositeChunk();
		}

    LIBCC_INLINE void SetFormat(const _Char* s)
		{
			if(s == 0)
			{
				Clear();
				return;
			}

			m_pos = 0;
			m_Format = s;
			m_Composite.clear();
			m_Composite.reserve(m_Format.size());
			BuildCompositeChunk();
		}

    template<typename CharX>
		LIBCC_INLINE void SetFormat(const std::basic_string<CharX>& s)
		{
			m_pos = 0;
			ConvertString(s, m_Format);
			m_Composite.clear();
			m_Composite.reserve(m_Format.size());
			BuildCompositeChunk();
		}

#ifdef WIN32
    // assign from stringtable resource
    LIBCC_INLINE void SetFormat(HINSTANCE hModule, UINT stringID)
		{
			m_pos = 0;
			if(LoadStringX(hModule, stringID, m_Format))
			{
				m_Composite.clear();
				m_Composite.reserve(m_Format.size());
				BuildCompositeChunk();
			}
		}

		LIBCC_INLINE void SetFormat(UINT stringID)
		{
			m_pos = 0;
			if(LoadStringX(GetModuleHandle(NULL), stringID, m_Format))
			{
				m_Composite.clear();
				m_Composite.reserve(m_Format.size());
				BuildCompositeChunk();
			}
		}
#endif

		// "GET" methods
    LIBCC_INLINE const _String Str() const
		{
			return m_Composite;
		}
    LIBCC_INLINE const _Char* CStr() const
		{
			return m_Composite.c_str();
		}
#if CCSTR_OPTION_AUTOCAST == 1
		LIBCC_INLINE operator _String() const
		{
			return m_Composite;
		}
    LIBCC_INLINE operator const _Char*() const
		{
			return m_Composite.c_str();
		}
#endif

    // POINTER - NOT portable when unsigned long != pointer size. -----------------------------
    template<typename T>
    LIBCC_INLINE _This& p(const T* v)
		{
			m_Composite.push_back('0');
			m_Composite.push_back('x');
			unsigned long temp = *(reinterpret_cast<unsigned long*>(&v));
			return ul<16, 8>(temp);// treat it as an unsigned number
		}

    // CHARACTER (count) -----------------------------
    template<typename T>
    LIBCC_INLINE _This& c(T v)
		{
			m_Composite.push_back(static_cast<_Char>(v));
			BuildCompositeChunk();
			return *this;
		}

    template<typename T>
    LIBCC_INLINE _This& c(T v, size_t count)
		{
			m_Composite.reserve(m_Composite.size() + count);
			for(; count > 0; --count)
			{
				m_Composite.push_back(static_cast<_Char>(v));
			}
			BuildCompositeChunk();
			return *this;
		}

    // STRING (maxlen) -----------------------------
    template<size_t MaxLen>
		LIBCC_INLINE _This& s(const _Char* s)
		{
			if(s && MaxLen) m_Composite.append(s, MaxLen);
			BuildCompositeChunk();
			return *this;
		}

    LIBCC_INLINE _This& s(const _Char* s, size_t MaxLen)
		{
			if(s) m_Composite.append(s, MaxLen);
			BuildCompositeChunk();
			return *this;
		}

    LIBCC_INLINE _This& s(const _Char* s)
		{
			if(s) m_Composite.append(s);
			BuildCompositeChunk();
			return *this;
		}

    template<size_t MaxLen>
		LIBCC_INLINE _This& s(const _String& s)
		{
			if(MaxLen) m_Composite.append(s, 0, MaxLen);
			BuildCompositeChunk();
			return *this;
		}

    LIBCC_INLINE _This& s(const _String& s, size_t MaxLen)
		{
			m_Composite.append(s, 0, MaxLen);
			BuildCompositeChunk();
			return *this;
		}

    LIBCC_INLINE _This& s(const _String& s)
		{
			m_Composite.append(s);
			BuildCompositeChunk();
			return *this;
		}

		// now all that but in foreign char types
    template<typename aChar>
    LIBCC_INLINE _This& s(const aChar* foreign)
		{
			if(foreign)
			{
				_String native;
				ConvertString(foreign, native);
				return s(native);
			}
			BuildCompositeChunk();
			return *this;
		}

		/*
			dilemma here is... do we convert foreign to native first, or truncate the string first? if i want
			to support localized strings correctly i should convert first. but that may mean potentially creating
			copies of huge strings when maxlen might be very small. i figure that's a rare enough case that i
			should go for accuracy.
		*/
    template<size_t MaxLen, typename aChar>
    LIBCC_INLINE _This& s(const aChar* foreign)
		{
			if(MaxLen && foreign)
			{
				_String native;
				ConvertString(foreign, native);
				return s<MaxLen>(native);
			}
			BuildCompositeChunk();
			return *this;
		}

    template<typename aChar>
    LIBCC_INLINE _This& s(const aChar* foreign, size_t MaxLen)
		{
			if(foreign)
			{
				_String native;
				ConvertString(foreign, native);
				return s(native, MaxLen);
			}
			BuildCompositeChunk();
			return *this;
		}
    
		template<typename aChar, typename aTraits, typename aAlloc>
    LIBCC_INLINE _This& s(const std::basic_string<aChar, aTraits, aAlloc>& x)
		{
			_String native;
			ConvertString(x, native);
			return s(native);
		}
    
		template<size_t MaxLen, typename aChar, typename aTraits, typename aAlloc>
    LIBCC_INLINE _This& s(const std::basic_string<aChar, aTraits, aAlloc>& x)
		{
			if(MaxLen)
			{
				_String native;
				ConvertString(x, native);
				return s<MaxLen>(native);
			}
			BuildCompositeChunk();
			return *this;
		}
    
		template<typename aChar, typename aTraits, typename aAlloc>
    LIBCC_INLINE _This& s(const std::basic_string<aChar, aTraits, aAlloc>& x, size_t MaxLen)
		{
			_String native;
			ConvertString(x, native);
			return s(native, MaxLen);
		}
		
		LIBCC_INLINE _This& NewLine()
		{
			AppendNewLine(m_Composite);
			BuildCompositeChunk();
			return *this;
		}
		
		LIBCC_INLINE _This& NewParagraph()
		{
			AppendNewParagraph(m_Composite);
			BuildCompositeChunk();
			return *this;
		}

    // QUOTED STRINGS (maxlen)
    template<size_t MaxLen>
    LIBCC_INLINE _This& qs(const _Char* s)
		{
			if(MaxLen && s)
			{
				m_Composite.push_back(OpenQuote);
				if(MaxLen >= 3)
				{
					m_Composite.append(s, MaxLen - 2);
					m_Composite.push_back(CloseQuote);
				}
				else if(MaxLen == 2)
				{
					m_Composite.push_back(CloseQuote);
				}
			}
			BuildCompositeChunk();
			return *this;
		}

    LIBCC_INLINE _This& qs(const _Char* s, size_t MaxLen)
		{
			if(MaxLen && s)
			{
				m_Composite.push_back(OpenQuote);
				if(MaxLen >= 3)
				{
					m_Composite.append(s, MaxLen - 2);
					m_Composite.push_back(CloseQuote);
				}
				else if(MaxLen == 2)
				{
					m_Composite.push_back(CloseQuote);
				}
			}
			BuildCompositeChunk();
			return *this;
		}

    LIBCC_INLINE _This& qs(const _Char* s)
		{
			if(s)
			{
				m_Composite.push_back(OpenQuote);
				m_Composite.append(s);
				m_Composite.push_back(CloseQuote);
			}
			BuildCompositeChunk();
			return *this;
		}

		template<size_t MaxLen>
    LIBCC_INLINE _This& qs(const _String& s)
		{
			if(MaxLen)
			{
				m_Composite.push_back(OpenQuote);
				if(MaxLen >= 3)
				{
					m_Composite.append(s, 0, MaxLen - 2);
					m_Composite.push_back(CloseQuote);
				}
				else if(MaxLen == 2)
				{
					m_Composite.push_back(CloseQuote);
				}
			}
			BuildCompositeChunk();
			return *this;
		}

    LIBCC_INLINE _This& qs(const _String& s, size_t MaxLen)
		{
			if(MaxLen > 0)
			{
				m_Composite.push_back(OpenQuote);
				if(MaxLen >= 3)
				{
					m_Composite.append(s, 0, MaxLen - 2);
					m_Composite.push_back(CloseQuote);
				}
				else if(MaxLen == 2)
				{
					m_Composite.push_back(CloseQuote);
				}
			}
			BuildCompositeChunk();
			return *this;
		}

    LIBCC_INLINE _This& qs(const _String& s)
		{
			m_Composite.push_back(OpenQuote);
			m_Composite.append(s);
			m_Composite.push_back(CloseQuote);
			BuildCompositeChunk();
			return *this;
		}

		// (now the same stuff but with foreign characters)
    template<typename aChar>
    LIBCC_INLINE _This& qs(const aChar* foreign)
		{
			if(foreign)
			{
				_String native;
				ConvertString(foreign, native);
				return qs(native);
			}
			BuildCompositeChunk();
			return *this;
		}

    template<size_t MaxLen, typename aChar>
    LIBCC_INLINE _This& qs(const aChar* foreign)
		{
			if(MaxLen && foreign)
			{
				_String native;
				ConvertString(foreign, native);
				return qs<MaxLen>(native);
			}
			BuildCompositeChunk();
			return *this;
		}

    template<typename aChar>
    LIBCC_INLINE _This& qs(const aChar* foreign, size_t MaxLen)
		{
			if(foreign)
			{
				_String native;
				ConvertString(foreign, native);
				return qs(native, MaxLen);
			}
			BuildCompositeChunk();
			return *this;
		}

    template<typename aChar, typename aTraits, typename aAlloc>
    LIBCC_INLINE _This& qs(const std::basic_string<aChar, aTraits, aAlloc>& x)
		{
			_String native;
			ConvertString(x, native);
			return qs(native);
		}

    template<size_t MaxLen, typename aChar, typename aTraits, typename aAlloc>
    LIBCC_INLINE _This& qs(const std::basic_string<aChar, aTraits, aAlloc>& x)
		{
			_String native;
			ConvertString(x, native);
			return qs<MaxLen>(native);
		}

    template<typename aChar, typename aTraits, typename aAlloc>
    LIBCC_INLINE _This& qs(const std::basic_string<aChar, aTraits, aAlloc>& x, size_t MaxLen)
		{
			_String native;
			ConvertString(x, native);
			return qs(native, MaxLen);
		}

    // UNSIGNED LONG -----------------------------
    template<size_t Base, size_t Width, _Char PadChar>
    LIBCC_INLINE _This& ul(unsigned long n)
		{
			const size_t BufferSize = _BufferSizeNeededInteger<Width, unsigned long>::Value;
			_Char buf[BufferSize];
			_Char* p = buf + BufferSize - 1;
			*p = 0;
			return s(_UnsignedNumberToString<Base, Width, PadChar>(p, n));
		}

    template<size_t Base, size_t Width>
    LIBCC_INLINE _This& ul(unsigned long n)
		{
			return ul<Base, Width, '0'>(n);
		}
    
		template<size_t Base>
    LIBCC_INLINE _This& ul(unsigned long n)
		{
			return ul<Base, 0, '0'>(n);
		}

    LIBCC_INLINE _This& ul(unsigned long n)
		{
			return ul<10, 0, '0'>(n);
		}

    LIBCC_INLINE _This& ul(unsigned long n, size_t Base, size_t Width = 0, _Char PadChar = '0')
		{
			const size_t BufferSize = _RuntimeBufferSizeNeededInteger<unsigned long>(Width);
			_Char* buf = (_Char*)_alloca(BufferSize * sizeof(_Char));
			_Char* p = buf + BufferSize - 1;
			*p = 0;
			return s(_RuntimeUnsignedNumberToString<unsigned long>(p, n, Base, Width, PadChar));
		}

    // SIGNED LONG -----------------------------
    template<size_t Base, size_t Width, _Char PadChar, bool ForceShowSign>
    LIBCC_INLINE _This& l(signed long n)
		{
			const size_t BufferSize = _BufferSizeNeededInteger<Width, signed long>::Value;
			_Char buf[BufferSize];
			_Char* p = buf + BufferSize - 1;
			*p = 0;
			return s(_SignedNumberToString<Base, Width, PadChar, ForceShowSign>(p, n));
		}

    template<size_t Base, size_t Width, _Char PadChar>
    LIBCC_INLINE _This& l(signed long n)
		{
			return l<Base, Width, PadChar, false>(n);
		}

    template<size_t Base, size_t Width>
    LIBCC_INLINE _This& l(signed long n)
		{
			return l<Base, Width, '0', false>(n);
		}

    template<size_t Base>
    LIBCC_INLINE _This& l(signed long n)
		{
			return l<Base, 0, '0', false>(n);
		}

    LIBCC_INLINE _This& l(signed long n)
		{
			return l<10, 0, '0', false>(n);
		}

    LIBCC_INLINE _This& l(signed long n, size_t Base, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false)
		{
			const size_t BufferSize = _RuntimeBufferSizeNeededInteger<unsigned long>(Width);
			_Char* buf = (_Char*)_alloca(BufferSize * sizeof(_Char));
			_Char* p = buf + BufferSize - 1;
			*p = 0;
			return s(_RuntimeSignedNumberToString(p, n, Base, Width, PadChar, ForceShowSign));
		}

    // UNSIGNED INT (just stubs for ul()) -----------------------------
    template<size_t Base, size_t Width, _Char PadChar>
    _This& ui(unsigned long n)
    {
      return ul<Base, Width, PadChar>(n);
    }
    template<size_t Base, size_t Width>
    _This& ui(unsigned long n)
    {
      return ul<Base, Width>(n);
    }
    template<size_t Base>
    _This& ui(unsigned long n)
    {
      return ul<Base, Width>(n);
    }
    _This& ui(unsigned long n)
    {
      return ul(n);
    }
    _This& ui(unsigned long n, size_t Base, size_t Width = 0, _Char PadChar = '0')
    {
      return ul(n, Base, Width, PadChar);
    }

    // SIGNED INT -----------------------------
    template<size_t Base, size_t Width, _Char PadChar, bool ForceShowSign>
    _This& i(signed long n)
    {
      return l<Base, Width, PadChar, ForceShowSign>(n);
    }
    template<size_t Base, size_t Width, _Char PadChar>
    _This& i(signed long n)
    {
      return l<Base, Width, PadChar>(n);
    }
    template<size_t Base, size_t Width>
    _This& i(signed long n)
    {
      return l<Base, Width>(n);
    }
    template<size_t Base>
    _This& i(signed long n)
    {
      return l<Base>(n);
    }
    _This& i(signed long n)
    {
      return l(n);
    }
    _This& i(signed long n, size_t Base, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false)
    {
      return l(n, Base, Width, PadChar, ForceShowSign);
    }

    // FLOAT ----------------------------- 3.14   [intwidth].[decwidth]
    // integralwidth is the MINIMUM digits.  Decimalwidth is the MAXIMUM digits.
    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign, size_t Base>
    LIBCC_INLINE _This& f(float val)
		{
	    return _AppendFloat<SinglePrecisionFloat, Base, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign>(val);
		}

    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign>
    LIBCC_INLINE _This& f(float val)
		{
	    return f<DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign, 10>(val);
		}

    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar>
    LIBCC_INLINE _This& f(float val)
		{
	    return f<DecimalWidthMax, IntegralWidthMin, PaddingChar, false, 10>(val);
		}

    template<size_t DecimalWidthMax, size_t IntegralWidthMin>
    LIBCC_INLINE _This& f(float val)
		{
	    return f<DecimalWidthMax, IntegralWidthMin, '0', false, 10>(val);
		}

    template<size_t DecimalWidthMax>
    LIBCC_INLINE _This& f(float val)
		{
	    return f<DecimalWidthMax, 1, '0', false, 10>(val);
		}

    LIBCC_INLINE _This& f(float val)
		{
	    return f<2, 1, '0', false, 10>(val);
		}

    LIBCC_INLINE _This& f(float val, size_t DecimalWidthMax = 2, size_t IntegralWidthMin = 1, _Char PaddingChar = '0', bool ForceSign = false, size_t Base = 10)
		{
	    return _RuntimeAppendFloat<SinglePrecisionFloat>(val, Base, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign);
		}

    // DOUBLE -----------------------------
    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign, size_t Base>
    LIBCC_INLINE _This& d(double val)
		{
	    return _AppendFloat<DoublePrecisionFloat, Base, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign>(val);
		}

    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign>
    LIBCC_INLINE _This& d(double val)
		{
	    return d<DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign, 10>(val);
		}

    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar>
    LIBCC_INLINE _This& d(double val)
		{
	    return d<DecimalWidthMax, IntegralWidthMin, PaddingChar, false, 10>(val);
		}

    template<size_t DecimalWidthMax, size_t IntegralWidthMin>
    LIBCC_INLINE _This& d(double val)
		{
	    return d<DecimalWidthMax, IntegralWidthMin, '0', false, 10>(val);
		}

    template<size_t DecimalWidthMax>
    LIBCC_INLINE _This& d(double val)
		{
	    return d<DecimalWidthMax, 1, '0', false, 10>(val);
		}

    LIBCC_INLINE _This& d(double val)
		{
			return d<3, 1, '0', false, 10>(val);
		}

    LIBCC_INLINE _This& d(double val, size_t DecimalWidthMax = 3, size_t IntegralWidthMin = 1, _Char PaddingChar = '0', bool ForceSign = false, size_t Base = 10)
		{
	    return _RuntimeAppendFloat<DoublePrecisionFloat>(val, Base, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign);
		}

    // UNSIGNED INT 64 -----------------------------
    template<size_t Base, size_t Width, _Char PadChar>
    LIBCC_INLINE _This& ui64(unsigned __int64 n)
		{
			const size_t BufferSize = _BufferSizeNeededInteger<Width, unsigned __int64>::Value;
			_Char buf[BufferSize];
			_Char* p = buf + BufferSize - 1;
			*p = 0;
			return s(_UnsignedNumberToString<Base, Width, PadChar>(p, n));
		}

    template<size_t Base, size_t Width>
    LIBCC_INLINE _This& ui64(unsigned __int64 n)
		{
	    return ui64<Base, Width, '0'>(n);
		}

    template<size_t Base> 
    LIBCC_INLINE _This& ui64(unsigned __int64 n)
		{
	    return ui64<Base, 0, 0>(n);
		}

    LIBCC_INLINE _This& ui64(unsigned __int64 n)
		{
	    return ui64<10, 0, 0>(n);
		}

    LIBCC_INLINE _This& ui64(unsigned __int64 n, size_t Base, size_t Width = 0, _Char PadChar = '0')
		{
			const size_t BufferSize = _RuntimeBufferSizeNeededInteger<unsigned __int64>(Width);
			_Char* buf = (_Char*)_alloca(BufferSize * sizeof(_Char));
			_Char* p = buf + BufferSize - 1;
			*p = 0;
			return s(_RuntimeUnsignedNumberToString<unsigned __int64>(p, n, Base, Width, PadChar));
		}

    // SIGNED INT 64 -----------------------------
    template<size_t Base, size_t Width, _Char PadChar, bool ForceShowSign>
    LIBCC_INLINE _This& i64(signed __int64 n)
		{
			const size_t BufferSize = _BufferSizeNeededInteger<Width, unsigned __int64>::Value;
			_Char buf[BufferSize];
			_Char* p = buf + BufferSize - 1;
			*p = 0;
			return s(_SignedNumberToString<Base, Width, PadChar, ForceShowSign>(p, n));
		}

    template<size_t Base, size_t Width, _Char PadChar>
    LIBCC_INLINE _This& i64(__int64 n)
		{
	    return i64<Base, Width, PadChar, false>(n);
		}

    template<size_t Base, size_t Width>
    LIBCC_INLINE _This& i64(__int64 n)
		{
	    return i64<Base, Width, '0', false>(n);
		}

    template<size_t Base>
    LIBCC_INLINE _This& i64(__int64 n)
		{
	    return i64<Base, 0, 0, false>(n);
		}

    LIBCC_INLINE _This& i64(__int64 n)
		{
	    return i64<10, 0, 0, false>(n);
		}

    LIBCC_INLINE _This& i64(signed __int64 n, size_t Base = 10, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false)
		{
			const size_t BufferSize = _RuntimeBufferSizeNeededInteger<unsigned __int64>(Width);
			_Char* buf = (_Char*)_alloca(BufferSize * sizeof(_Char));
			_Char* p = buf + BufferSize - 1;
			*p = 0;
			return s(_RuntimeSignedNumberToString<signed __int64>(p, n, Base, Width, PadChar, ForceShowSign));
		}

    // GETLASTERROR() -----------------------------
#ifdef WIN32
    LIBCC_INLINE _This& gle(int code)
		{
			_String str;
			FormatMessageGLE(str, code);
			return s(str);
		}

    LIBCC_INLINE _This& gle()
		{
	    return gle(GetLastError());
		}
#endif

    // CONVENIENCE OPERATOR () -----------------------------
    _This& operator ()(int n, size_t Base = 10, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false)
    {
      return i(n, Base, Width, PadChar, ForceShowSign);
    }
    _This& operator ()(unsigned int n, size_t Base = 10, size_t Width = 0, _Char PadChar = '0')
    {
      return ui(n, Base, Width, PadChar);
    }
    _This& operator ()(__int64 n, size_t Base = 10, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false)
    {
      return i64(n, Base, Width, PadChar, ForceShowSign);
    }
    _This& operator ()(unsigned __int64 n, size_t Base = 10, size_t Width = 0, _Char PadChar = '0')
    {
      return ui64(n, Base, Width, PadChar);
    }
    _This& operator ()(float n, size_t DecimalWidthMax = 2, size_t IntegralWidthMin = 1, _Char PaddingChar = '0', bool ForceSign = false, size_t Base = 10)
    {
      return f(n, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign, Base);
    }
    _This& operator ()(double n, size_t DecimalWidthMax = 2, size_t IntegralWidthMin = 1, _Char PaddingChar = '0', bool ForceSign = false, size_t Base = 10)
    {
      return d(n, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign, Base);
    }
    _This& operator ()(char* n)
    {
      return s(n);
    }
    _This& operator ()(wchar_t* n)
    {
      return s(n);
    }
    _This& operator ()(const std::string& n)
    {
      return s(n);
    }
    _This& operator ()(const std::wstring& n)
    {
      return s(n);
    }

  private:
    LIBCC_INLINE _This& _RuntimeAppendZeroFloat(size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign)
		{
			// zero.
			// pre-decimal part.
			// "-----0"
			if(IntegralWidthMin > 0)
			{
				// append padding
				m_Composite.reserve(m_Composite.size() + IntegralWidthMin);
				for(size_t i = 1; i < IntegralWidthMin; ++ i)
				{
					m_Composite.push_back(static_cast<_Char>(PaddingChar));
				}
				// append the integral zero
				m_Composite.push_back('0');
			}
			if(DecimalWidthMax)
			{
				// if there are any decimal digits to set, then just append ".0"
				m_Composite.reserve(m_Composite.size() + 2);
				m_Composite.push_back('.');
				m_Composite.push_back('0');
			}
			BuildCompositeChunk();
			return *this;
		}

    template<typename FloatType>
    LIBCC_INLINE _This& _RuntimeAppendNormalizedFloat(FloatType& _f, size_t Base, size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign)
		{
			// how do we know how many chars we will use?  we don't right now.
			_Char* buf = reinterpret_cast<_Char*>(_alloca(sizeof(_Char) * (2200 + IntegralWidthMin + DecimalWidthMax)));
			long IntegralWidthLeft = static_cast<long>(IntegralWidthMin);
			_Char* middle = buf + 2100 - DecimalWidthMax;
			_Char* sIntPart = middle;
			_Char* sDecPart = middle;
			FloatType::Mantissa _int;// integer part raw value
			FloatType::Mantissa _dec;// decimal part raw value
			FloatType::Exponent exp = _f.GetExponent();// exponent raw value
			FloatType::Mantissa m = _f.GetMantissa();
			size_t DecBits;// how many bits out of the mantissa are used by the decimal part?

			const size_t BasicTypeBits = sizeof(FloatType::BasicType)*8;
			if((exp < FloatType::MantissaBits) && (exp > (FloatType::MantissaBits - BasicTypeBits)))
			{
				// write the integral (before decimal point) part.
				DecBits = _f.MantissaBits - exp;
				_int = m >> DecBits;// the integer part.
				_dec = m & (((FloatType::Mantissa)1 << DecBits) - 1);
				do
				{
					--IntegralWidthLeft;
					*(-- sIntPart) = DigitToChar(static_cast<unsigned char>(_int % Base));
					_int = _int / static_cast<FloatType::Mantissa>(Base);
				}
				while(_int);

				while(IntegralWidthLeft > 0)
				{
					*(-- sIntPart) = static_cast<_Char>(PaddingChar);
					IntegralWidthLeft --;
				}

				// write the after-decimal part.  here we basically do long division!
				// the decimal part is basically a fraction that we convert bases on.
				// since we need to deal with a number as large as the denominator, this will only work
				// when DecBits is less than 32 (for single precsion)
				if(DecimalWidthMax)
				{
					size_t DecimalWidthLeft = DecimalWidthMax;
					middle[0] = '.';
					FloatType::Mantissa denominator = (FloatType::Mantissa)1 << DecBits;// same as 'capacity'.
					FloatType::Mantissa& numerator(_dec);
					numerator *= static_cast<FloatType::Mantissa>(Base);
					FloatType::Mantissa digit;
					while(numerator && DecimalWidthLeft)
					{
						digit = numerator / denominator;// integer division
						// add the digit, and drill down into the remainder.
						*(++ sDecPart) = DigitToChar(static_cast<unsigned char>(digit % Base));
						numerator -= digit * denominator;
						numerator *= static_cast<FloatType::Mantissa>(Base);
						-- DecimalWidthLeft;
					}
				}
				else
				{
					middle[0] = 0;
				}
			}
			else
			{
				// We are here because doing conversions would take large numbers - too large to hold in
				// a InternalType integral.  So until i can come up with a cooler way to do it, i will
				// just do floating point divides and 

				// do the integral part just like a normal int.
				FloatType::This integerPart(_f);
				integerPart.RemoveDecimal();
				integerPart.AbsoluteValue();
				FloatType::BasicType fBase = static_cast<FloatType::BasicType>(Base);
				do
				{
					IntegralWidthLeft --;
					// at this point integerPart has no decimal and Base of course doesnt.
					*(-- sIntPart) = DigitToChar(static_cast<unsigned char>(fmod(integerPart.m_BasicVal, fBase)));
					integerPart.m_BasicVal /= Base;
					integerPart.RemoveDecimal();
				}
				while(integerPart.m_BasicVal > 0);

				while(IntegralWidthLeft > 0)
				{
					*(-- sIntPart) = static_cast<_Char>(PaddingChar);
					IntegralWidthLeft --;
				}

				// now the decimal part.
				if(DecimalWidthMax)
				{
					size_t DecimalWidthLeft = DecimalWidthMax;
					middle[0] = '.';
					FloatType::This val(_f);
					val.AbsoluteValue();
					// remove integer part.
					FloatType::This integerPart(val);
					integerPart.RemoveDecimal();
					val.m_BasicVal -= integerPart.m_BasicVal;
					do
					{
						DecimalWidthLeft --;
						val.m_BasicVal *= Base;
						// isolate the integral part
						integerPart.m_BasicVal = val.m_BasicVal;
						integerPart.RemoveDecimal();
						*(++ sDecPart) = DigitToChar(static_cast<unsigned char>(fmod(integerPart.m_BasicVal, fBase)));
						// use the integral part to leave only the decimal part.
						val.m_BasicVal -= integerPart.m_BasicVal;
					}
					while((val.m_BasicVal > 0) && DecimalWidthLeft);
				}
				else
				{
					middle[0] = 0;
				}
			}

			// display the sign
			if(_f.IsNegative())
			{
				*(-- sIntPart) = '-';
			}
			else if(ForceSign)
			{
				*(-- sIntPart) = '+';
			}

			// null terminate
			*(++ sDecPart) = 0;

			return s(sIntPart);
		}

    /*
      Converts any floating point (LibCC::IEEEFloat<>) number to a string, and appends it just like any other string.
    */
    template<typename FloatType>
    LIBCC_INLINE _This& _RuntimeAppendFloat(const FloatType& _f, size_t Base, size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign)
		{
			if(!(_f.m_val & _f.ExponentMask))
			{
				// exponont = 0.  that means its either zero or denormalized.
				if(_f.m_val & _f.MantissaMask)
				{
					// denormalized
					return s("Unsupported denormalized number");
				}
				else
				{
					// zero
					return _RuntimeAppendZeroFloat(DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign);
				}
			}
			else if((_f.m_val & _f.ExponentMask) == _f.ExponentMask)
			{
				// exponent = MAX.  either infinity or NAN.
				if(_f.IsPositiveInfinity())
				{
					return s("+Inf");
				}
				else if(_f.IsNegativeInfinity())
				{
					return s("-Inf");
				}
				else if(_f.IsQNaN())
				{
					return s("QNaN");
				}
				else if(_f.IsSNaN())
				{
					return s("SNaN");
				}
			}

			// normalized number.
			return _RuntimeAppendNormalizedFloat(_f, Base, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign);
		}

    template<typename FloatType, size_t Base, size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign>
    LIBCC_INLINE _This& _AppendFloat(const FloatType& _f)
		{
	    return _RuntimeAppendFloat<FloatType>(_f, Base, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign);
		}

    template<size_t Width, typename T>
    struct _BufferSizeNeededInteger
    {
      // sizeof(T) * 8 == how many bits to store the value.  considering
      // the smallest base supported is base 2 (binary), thats exactly how
      // many digits maximum for an integer type.  +1 for null terminator
      // this is basically max(size based on width, size based on sizeof())
      // and +1 for the sign.
      static const long Value = (sizeof(T) * 8) + 2 > (Width + 1) ? (sizeof(T) * 8) + 2 : (Width + 1);
    };

    template<typename T>
    LIBCC_INLINE long _RuntimeBufferSizeNeededInteger(size_t Width)
		{
	    return (sizeof(T) * 8) + 2 > (Width + 1) ? (sizeof(T) * 8) + 2 : (Width + 1);
		}

    // buf must point to a null terminator.  It is "pulled back" and the result is returned.
    // its simply faster to build the string in reverse order.
    template<typename T>
    LIBCC_INLINE static _Char* _RuntimeUnsignedNumberToString(_Char* buf, T num, size_t Base, size_t Width, _Char PaddingChar)
		{
			long PadRemaining = static_cast<long>(Width);
			_Char _PadChar = PaddingChar;
			do
			{
				PadRemaining --;
				*(--buf) = static_cast<_Char>(DigitToChar(static_cast<unsigned char>(num % Base)));
				num = num / static_cast<T>(Base);
			}
			while(num);

			while(PadRemaining-- > 0)
			{
				*(--buf) = _PadChar;
			}
			return buf;
		}

    template<size_t Base, size_t Width, _Char PaddingChar, typename T>
    LIBCC_INLINE static _Char* _UnsignedNumberToString(_Char* buf, T num)
		{
			if(Base < 2)
			{
				static _Char x[] = { 0 };
				return x;
			}
			long PadRemaining = static_cast<long>(Width);
			_Char _PadChar = PaddingChar;
			do
			{
				PadRemaining --;
				*(--buf) = static_cast<_Char>(DigitToChar(static_cast<unsigned char>(num % Base)));
				num = num / static_cast<T>(Base);
			}
			while(num);

			while(PadRemaining-- > 0)
			{
				*(--buf) = _PadChar;
			}
			return buf;
		}

    // same thing, but params can be set at runtime
    template<typename T>
    LIBCC_INLINE static _Char* _RuntimeSignedNumberToString(_Char* buf, T num, size_t Base, size_t Width, _Char PaddingChar, bool ForceSign)
		{
			if(Base < 2)
			{
				static _Char x[] = { 0 };
				return x;
			}
			if(num < 0)
			{
				buf = _RuntimeUnsignedNumberToString(buf, -num, Base, Width-1, PaddingChar);
				*(--buf) = '-';
			}
			else
			{
				if(ForceSign)
				{
					buf = _RuntimeUnsignedNumberToString(buf, num, Base, Width-1, PaddingChar);
					*(--buf) = '+';
				}
				else
				{
					buf = _RuntimeUnsignedNumberToString(buf, num, Base, Width, PaddingChar);
				}
			}
			return buf;
		}

    template<size_t Base, size_t Width, _Char PaddingChar, bool ForceSign, typename T>
    LIBCC_INLINE static _Char* _SignedNumberToString(_Char* buf, T num)
		{
			if(num < 0)
			{
				buf = _UnsignedNumberToString<Base, Width-1, PaddingChar, T>(buf, -num);
				*(--buf) = '-';
			}
			else
			{
				if(ForceSign)
				{
					buf = _UnsignedNumberToString<Base, Width-1, PaddingChar, T>(buf, num);
					*(--buf) = '+';
				}
				else
				{
					buf = _UnsignedNumberToString<Base, Width, PaddingChar, T>(buf, num);
				}
			}
			return buf;
		}

    // build composite as much as we can (until a replace-char)
    LIBCC_INLINE void BuildCompositeChunk()
		{
			// go from m_pos to the next insertion point
			bool bKeepGoing = true;
			while(bKeepGoing)
			{
				if(m_pos >= m_Format.size())
				{
					break;
				}
				else
				{
					_Char ch = m_Format[m_pos];
					switch(ch)
					{
					case EscapeChar:
						++ m_pos;
						if(m_pos < m_Format.size())
						{
							m_Composite.push_back(m_Format[m_pos]);
						}
						break;
					case NewlineChar:
						AppendNewLine(m_Composite);
						break;
					case ReplaceChar:
						// we are done.  the loop will advance the thing one more, then end.
						bKeepGoing = false;
						break;
					default:
						m_Composite.push_back(ch);
						break;
					}

					++ m_pos;
				}
			}
		}

    _String m_Format;// the original format string.  this plus arguments that are fed in is used to build m_Composite.
    _String m_Composite;// the "output" string - this is what we are building.
    typename _String::size_type m_pos;// 0-based offset into m_Format that points to the first character not in m_Composite.
  
# ifdef WIN32
		// a couple functions here are copied from winapi for local use.
		template<typename Traits, typename Alloc>
		LIBCC_INLINE static void FormatMessageGLE(std::basic_string<wchar_t, Traits, Alloc>& out, int code)
		{
			wchar_t* lpMsgBuf(0);
			FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
				0, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL);
			if(lpMsgBuf)
			{
				out = lpMsgBuf;
				LocalFree(lpMsgBuf);
			}
			else
			{
				out = L"Unknown error: ";
				wchar_t temp[50];
				_itow(code, temp, 50);
				out.append(temp);
			}
		}

		template<typename Char, typename Traits, typename Alloc>
		inline static void FormatMessageGLE(std::basic_string<Char, Traits, Alloc>& out, int code)
		{
			std::wstring s;
			FormatMessageGLE(s, code);
			ConvertString(s, out);
			return;
		}

		template<typename Traits, typename Alloc>
		LIBCC_INLINE static bool LoadStringX(HINSTANCE hInstance, UINT stringID, std::basic_string<wchar_t, Traits, Alloc>& out)
		{
			static const int StaticBufferSize = 1024;
			static const int MaximumAllocSize = 5242880;// don't attempt loading strings larger than 10 megs
			bool r = false;
			wchar_t temp[StaticBufferSize];// start with fast stack buffer
			// LoadString returns the # of chars copied, not including the null terminator
			if(LoadStringW(hInstance, stringID, temp, StaticBufferSize) < (StaticBufferSize - 1))
			{
				out = temp;
				r = true;
			}
			else
			{
				// we loaded up the maximum size; the string was probably truncated.
				int size = StaticBufferSize * 2;// this # is in chars, not bytes
				while(1)
				{
					// allocate a buffer.
					if(size > MaximumAllocSize)
					{
						// failed... too large of a string.
						break;
					}
					wchar_t* buf = static_cast<wchar_t*>(HeapAlloc(GetProcessHeap(), 0, size * sizeof(wchar_t)));
					if(LoadStringW(hInstance, stringID, buf, size) < (size - 1))
					{
						// got what we wanted.
						out = buf;
						HeapFree(GetProcessHeap(), 0, buf);
						r = true;
						break;
					}
					HeapFree(GetProcessHeap(), 0, buf);
					size <<= 1;// double the amount to allocate
				}
			}
			return r;
		}

		template<typename Char, typename Traits, typename Alloc>
		inline bool static LoadStringX(HINSTANCE hInstance, UINT stringID, std::basic_string<Char, Traits, Alloc>& out)
		{
			bool r = false;
			std::wstring ws;
			if(LoadStringX(hInstance, stringID, ws))
			{
				r = true;
				ConvertString(ws, out);
			}
			return r;
		}
# endif
  };

  typedef FormatX<char, std::char_traits<char>, std::allocator<char> > FormatA;
  typedef FormatX<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > FormatW;
  typedef FormatX<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > Format;
}

#pragma warning(pop)
