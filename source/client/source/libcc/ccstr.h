/*
  Last updated May 18, 2005 Carl Corcoran

  (c) 2004-2005 Carl Corcoran, carl@ript.net
  http://carl.ript.net/stringformat/
  http://carl.ript.net/wp
  http://mantis.winprog.org/
  http://svn.winprog.org/personal/carl/stringformat

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

#include "float.h"
#include "winapi.h"
#include "StringBase.h"

#include <string>
#include <tchar.h>
#include <malloc.h>// for alloca()
#include <math.h>// for fmod()

namespace LibCC
{
  // FormatX class declaration -----------------------------------------------------------------------------------
  template<typename Ch, typename Traits, typename Alloc>
  class FormatX
  {
  public:
    typedef Ch _Char;
    typedef Traits _Traits;
    typedef Alloc _Alloc;
    typedef std::basic_string<_Char, _Traits, _Alloc> _String;
    typedef FormatX<_Char, _Traits, _Alloc> _This;

    static const char ReplaceChar = '%';
    static const char EscapeChar = '^';
    static const char NewlineChar = '|';

    CCSTR_INLINE const _String Str() const;
    CCSTR_INLINE const _Char* CStr() const;

#if CCSTR_OPTION_AUTOCAST == 1
    CCSTR_INLINE operator _String() const;
    CCSTR_INLINE operator const _Char*() const;
#endif

    // Construction / Assignment
    CCSTR_INLINE FormatX();
    CCSTR_INLINE explicit FormatX(const _String& s);
    CCSTR_INLINE explicit FormatX(const _Char* s);
    CCSTR_INLINE FormatX(const _This& r);
    template<typename CharX>
    explicit inline FormatX(const CharX* s);

#ifdef CCSTR_WIN32
    // construct from stringtable resource
    CCSTR_INLINE FormatX(HINSTANCE hModule, UINT stringID);
    CCSTR_INLINE FormatX(UINT stringID);
#endif
    template<typename CharX>
    CCSTR_INLINE void SetFormat(const CharX* s);
    CCSTR_INLINE void SetFormat(const _String& s);
    CCSTR_INLINE void SetFormat(const _Char* s);
#ifdef CCSTR_WIN32
    // assign from stringtable resource
    CCSTR_INLINE void SetFormat(HINSTANCE hModule, UINT stringID);
    CCSTR_INLINE void SetFormat(UINT stringID);
#endif

    // POINTER -----------------------------
    template<typename T>
    CCSTR_INLINE _This& p(const T* v);

    // CHARACTER -----------------------------
    template<typename T>
    CCSTR_INLINE _This& c(T v);
    template<typename T>
    CCSTR_INLINE _This& c(T v, size_t count);

    // STRING -----------------------------
    template<size_t MaxLen>
    CCSTR_INLINE _This& s(const _Char* s);
    CCSTR_INLINE _This& s(const _Char* s, size_t MaxLen);
    CCSTR_INLINE _This& s(const _Char* s);
    template<typename aChar>
    CCSTR_INLINE _This& s(const aChar* foreign);
    template<size_t MaxLen, typename aChar>
    CCSTR_INLINE _This& s(const aChar* foreign);
    template<typename aChar>
    CCSTR_INLINE _This& s(const aChar* foreign, size_t MaxLen);
    template<typename aChar, typename aTraits, typename aAlloc>
    CCSTR_INLINE _This& s(const std::basic_string<aChar, aTraits, aAlloc>& x);
    template<size_t MaxLen, typename aChar, typename aTraits, typename aAlloc>
    CCSTR_INLINE _This& s(const std::basic_string<aChar, aTraits, aAlloc>& x);
    template<typename aChar, typename aTraits, typename aAlloc>
    CCSTR_INLINE _This& s(const std::basic_string<aChar, aTraits, aAlloc>& x, size_t MaxLen);
    CCSTR_INLINE _This& NewLine();

    // QUOTED STRINGS
    template<size_t MaxLen>
    CCSTR_INLINE _This& qs(const _Char* s);
    CCSTR_INLINE _This& qs(const _Char* s, size_t MaxLen);
    CCSTR_INLINE _This& qs(const _Char* s);
    template<typename aChar>
    CCSTR_INLINE _This& qs(const aChar* foreign);
    template<size_t MaxLen, typename aChar>
    CCSTR_INLINE _This& qs(const aChar* foreign);
    template<typename aChar>
    CCSTR_INLINE _This& qs(const aChar* foreign, size_t MaxLen);
    template<typename aChar, typename aTraits, typename aAlloc>
    CCSTR_INLINE _This& qs(const std::basic_string<aChar, aTraits, aAlloc>& x);
    template<size_t MaxLen, typename aChar, typename aTraits, typename aAlloc>
    CCSTR_INLINE _This& qs(const std::basic_string<aChar, aTraits, aAlloc>& x);
    template<typename aChar, typename aTraits, typename aAlloc>
    CCSTR_INLINE _This& qs(const std::basic_string<aChar, aTraits, aAlloc>& x, size_t MaxLen);

    // UNSIGNED LONG -----------------------------
    template<size_t Base, size_t Width, _Char PadChar>
    CCSTR_INLINE _This& ul(unsigned long n);
    template<size_t Base, size_t Width>
    CCSTR_INLINE _This& ul(unsigned long n);
    template<size_t Base>
    CCSTR_INLINE _This& ul(unsigned long n);
    CCSTR_INLINE _This& ul(unsigned long n);
    CCSTR_INLINE _This& ul(unsigned long n, size_t Base, size_t Width = 0, _Char PadChar = '0');

    // SIGNED LONG -----------------------------
    template<size_t Base, size_t Width, _Char PadChar, bool ForceShowSign>
    CCSTR_INLINE _This& l(signed long n);
    template<size_t Base, size_t Width, _Char PadChar>
    CCSTR_INLINE _This& l(signed long n);
    template<size_t Base, size_t Width>
    CCSTR_INLINE _This& l(signed long n);
    template<size_t Base>
    CCSTR_INLINE _This& l(signed long n);
    CCSTR_INLINE _This& l(signed long n);
    CCSTR_INLINE _This& l(signed long n, size_t Base, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false);

    // UNSIGNED INT (just stubs for ul()) -----------------------------
    template<size_t Base, size_t Width, _Char PadChar>
    _This& ui(unsigned int n)
    {
      return ul<Base, Width, PadChar>(static_cast<long>(n));
    }
    template<size_t Base, size_t Width>
    _This& ui(unsigned int n)
    {
      return ul<Base, Width>(static_cast<long>(n));
    }
    template<size_t Base>
    _This& ui(unsigned int n)
    {
      return ul<Base>(static_cast<long>(n));
    }
    _This& ui(unsigned int n)
    {
      return ul(static_cast<long>(n));
    }
    _This& ui(unsigned int n, size_t Base, size_t Width = 0, _Char PadChar = '0')
    {
      return ul(static_cast<long>(n), Base, Width, PadChar);
    }

    // size_t (just stubs for ul()) -----------------------------
    template<size_t Base, size_t Width, _Char PadChar>
    _This& st(size_t n)
    {
      return ul<Base, Width, PadChar>(static_cast<long>(n));
    }
    template<size_t Base, size_t Width>
    _This& st(size_t n)
    {
      return ul<Base, Width>(static_cast<long>(n));
    }
    template<size_t Base>
    _This& st(size_t n)
    {
      return ul<Base>(static_cast<long>(n));
    }
    _This& st(size_t n)
    {
      return ul(static_cast<long>(n));
    }
    _This& st(size_t n, size_t Base, size_t Width = 0, _Char PadChar = '0')
    {
      return ul(static_cast<long>(n), Base, Width, PadChar);
    }

    // SIGNED INT -----------------------------
    template<size_t Base, size_t Width, _Char PadChar, bool ForceShowSign>
    _This& i(signed int n)
    {
      return l<Base, Width, PadChar, ForceShowSign>(static_cast<long>(n));
    }
    template<size_t Base, size_t Width, _Char PadChar>
    _This& i(signed int n)
    {
      return l<Base, Width, PadChar>(static_cast<long>(n));
    }
    template<size_t Base, size_t Width>
    _This& i(signed int n)
    {
      return l<Base, Width>(static_cast<long>(n));
    }
    template<size_t Base>
    _This& i(signed int n)
    {
      return l<Base>(static_cast<long>(n));
    }
    _This& i(signed int n)
    {
      return l(static_cast<long>(n));
    }
    _This& i(signed int n, size_t Base, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false)
    {
      return l(static_cast<long>(n), Base, Width, PadChar, ForceShowSign);
    }

    // FLOAT ----------------------------- 3.14   [intwidth].[decwidth]
    // integralwidth is the MINIMUM digits.  Decimalwidth is the MAXIMUM digits.
    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign, size_t Base>
    CCSTR_INLINE _This& f(float val);
    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign>
    CCSTR_INLINE _This& f(float val);
    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar>
    CCSTR_INLINE _This& f(float val);
    template<size_t DecimalWidthMax, size_t IntegralWidthMin>
    CCSTR_INLINE _This& f(float val);
    template<size_t DecimalWidthMax>
    CCSTR_INLINE _This& f(float val);
    CCSTR_INLINE _This& f(float val, size_t DecimalWidthMax = 2, size_t IntegralWidthMin = 1, _Char PaddingChar = '0', bool ForceSign = false, size_t Base = 10);

    // DOUBLE -----------------------------
    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign, size_t Base>
    CCSTR_INLINE _This& d(double val);
    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign>
    CCSTR_INLINE _This& d(double val);
    template<size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar>
    CCSTR_INLINE _This& d(double val);
    template<size_t DecimalWidthMax, size_t IntegralWidthMin>
    CCSTR_INLINE _This& d(double val);
    template<size_t DecimalWidthMax>
    CCSTR_INLINE _This& d(double val);
    CCSTR_INLINE _This& d(double val, size_t DecimalWidthMax = 3, size_t IntegralWidthMin = 1, _Char PaddingChar = '0', bool ForceSign = false, size_t Base = 10);

    // UNSIGNED INT 64 -----------------------------
    template<size_t Base, size_t Width, _Char PadChar>
    CCSTR_INLINE _This& ui64(unsigned __int64 n);
    template<size_t Base, size_t Width>
    CCSTR_INLINE _This& ui64(unsigned __int64 n);
    template<size_t Base> 
    CCSTR_INLINE _This& ui64(unsigned __int64 n);
    CCSTR_INLINE _This& ui64(unsigned __int64 n);
    CCSTR_INLINE _This& ui64(unsigned __int64 n, size_t Base, size_t Width = 0, _Char PadChar = '0');

    // SIGNED INT 64 -----------------------------
    template<size_t Base, size_t Width, _Char PadChar, bool ForceShowSign>
    CCSTR_INLINE _This& i64(signed __int64 n);
    template<size_t Base, size_t Width, _Char PadChar>
    CCSTR_INLINE _This& i64(__int64 n);
    template<size_t Base, size_t Width>
    CCSTR_INLINE _This& i64(__int64 n);
    template<size_t Base>
    CCSTR_INLINE _This& i64(__int64 n);
    CCSTR_INLINE _This& i64(__int64 n);
    CCSTR_INLINE _This& i64(signed __int64 n, size_t Base = 10, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false);

    // GETLASTERROR() -----------------------------
#ifdef CCSTR_WIN32
    CCSTR_INLINE _This& gle(int code);
    CCSTR_INLINE _This& gle();
#endif

    // CONVENIENCE OPERATOR () -----------------------------
    _This& operator ()(int n, size_t Base = 10, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false)
    {
      return i(n, Base, Width, PadChar, ForceShowSign);
    }
    _This& operator ()(size_t n, size_t Base = 10, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false)
    {
      return st(n, Base, Width, PadChar, ForceShowSign);
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
    CCSTR_INLINE _This& _RuntimeAppendZeroFloat(size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign);
    template<typename FloatType>
    CCSTR_INLINE _This& _RuntimeAppendNormalizedFloat(FloatType& _f, size_t Base, size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign);

    /*
      Converts any floating point (LibCC::IEEEFloat<>) number to a string, and appends it just like any other string.
    */
    template<typename FloatType>
    CCSTR_INLINE _This& _RuntimeAppendFloat(const FloatType& _f, size_t Base, size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign);
    template<typename FloatType, size_t Base, size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign>
    CCSTR_INLINE _This& _AppendFloat(const FloatType& _f);

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
    CCSTR_INLINE long _RuntimeBufferSizeNeededInteger(size_t Width);

    // buf must point to a null terminator.  It is "pulled back" and the result is returned.
    // its simply faster to build the string in reverse order.
    template<typename T>
    CCSTR_INLINE static _Char* _RuntimeUnsignedNumberToString(_Char* buf, T num, size_t Base, size_t Width, _Char PaddingChar);
    template<size_t Base, size_t Width, _Char PaddingChar, typename T>
    CCSTR_INLINE static _Char* _UnsignedNumberToString(_Char* buf, T num);
    // same thing, but params can be set at runtime
    template<typename T>
    CCSTR_INLINE static _Char* _RuntimeSignedNumberToString(_Char* buf, T num, size_t Base, size_t Width, _Char PaddingChar, bool ForceSign);
    template<size_t Base, size_t Width, _Char PaddingChar, bool ForceSign, typename T>
    CCSTR_INLINE static _Char* _SignedNumberToString(_Char* buf, T num);

    // build composite as much as we can (until a replace-char)
    CCSTR_INLINE void BuildCompositeChunk();

    _String m_Format;// the original format string.  this plus arguments that are fed in is used to build m_Composite.
    _String m_Composite;// the "output" string - this is what we are building.
    typename _String::size_type m_pos;// 0-based offset into m_Format that points to the first character not in m_Composite.
  };

  typedef FormatX<char, std::char_traits<char>, std::allocator<char> > FormatA;
  typedef FormatX<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > FormatW;
  typedef FormatX<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > Format;
}

#include "ccstrimpl.h"
