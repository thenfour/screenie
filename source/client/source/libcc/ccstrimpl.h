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

// This is an inline h file for implementation.  Do not #include it.

#pragma warning(push)

/*
  Disable "conditional expression is constant".  This code has conditions
  on integral template params, which are by design constant.  This is a lame
  warning.
*/
#pragma warning(disable:4127)

#ifdef CCSTR_WIN32
# include <windows.h>// for GetLastError()
#endif

namespace LibCC
{
#define THIS_T FormatX<Char__, Traits__, Alloc__>

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE const typename THIS_T::_String THIS_T::Str() const
  {
    return m_Composite;
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE const Char__* THIS_T::CStr() const
  {
    return m_Composite.c_str();
  }

#if CCSTR_OPTION_AUTOCAST == 1
  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T::operator typename THIS_T::_String() const
  {
    return m_Composite;
  }
  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T::operator const typename THIS_T::_Char*() const
  {
    return m_Composite.c_str();
  }
#endif

  // Construction
  template<typename Char__, typename Traits__, typename Alloc__>
  THIS_T::FormatX() :
    m_pos(0)
  {
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  THIS_T::FormatX(const _String& s) :
    m_Format(s),
    m_pos(0)
  {
    m_Composite.reserve(m_Format.size());
    BuildCompositeChunk();
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  THIS_T::FormatX(const _Char* s) :
    m_Format(s),
    m_pos(0)
  {
    if(!s) return;
    m_Composite.reserve(m_Format.size());
    BuildCompositeChunk();
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  THIS_T::FormatX(const _This& r) :
    m_pos(r.m_pos),
    m_Format(r.m_Format),
    m_Composite(r.m_Composite)
  {
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<typename CharX>
  CCSTR_INLINE THIS_T::FormatX(const CharX* s) :
    m_pos(0)
  {
    if(!s) return;
    StringCopy(m_Format, s);
    BuildCompositeChunk();
  }

#ifdef CCSTR_WIN32
  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T::FormatX(HINSTANCE hModule, UINT stringID) :
    m_pos(0)
  {
    if(LoadStringX(hModule, stringID, m_Format))
    {
      m_Composite.reserve(m_Format.size());
      BuildCompositeChunk();
    }
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T::FormatX(UINT stringID) :
    m_pos(0)
  {
    if(LoadStringX(GetModuleHandle(NULL), stringID, m_Format))
    {
      m_Composite.reserve(m_Format.size());
      BuildCompositeChunk();
    }
  }
#endif

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE void THIS_T::SetFormat(const _String& s)
  {
    m_pos = 0;
    m_Format = s;
    m_Composite.reserve(m_Format.size());
    BuildCompositeChunk();
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE void THIS_T::SetFormat(const _Char* s)
  {
    if(!s)
    {
      m_pos = 0;
      m_Format.clear();
      m_Composite.clear();
      return;
    }

    m_pos = 0;
    m_Format = s;
    m_Composite.reserve(m_Format.size());
    BuildCompositeChunk();
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<typename CharX>
  CCSTR_INLINE void THIS_T::SetFormat(const CharX* s)
  {
    if(!s)
    {
      m_pos = 0;
      m_Format.clear();
      m_Composite.clear();
    }

    m_pos = 0;
    StringCopy(m_Format, s);
    m_Composite.reserve(m_Format.size());
    BuildCompositeChunk();
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE void THIS_T::SetFormat(HINSTANCE hModule, UINT stringID)
  {
    m_pos = 0;
    if(LoadStringX(hModule, stringID, m_Format))
    {
      m_Composite.reserve(m_Format.size());
      BuildCompositeChunk();
    }
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE void THIS_T::SetFormat(UINT stringID)
  {
    m_pos = 0;
    if(LoadStringX(GetModuleHandle(NULL), stringID, m_Format))
    {
      m_Composite.reserve(m_Format.size());
      BuildCompositeChunk();
    }
  }

  // POINTER -----------------------------
  template<typename Char__, typename Traits__, typename Alloc__>
  template<typename T>
  CCSTR_INLINE THIS_T& THIS_T::p(const T* v)
  {
    m_Composite.push_back('0');
    m_Composite.push_back('x');
    unsigned long temp = *(reinterpret_cast<unsigned long*>(&v));
    return ul<16, 8>(temp);// treat it as an unsigned number
  }

  // CHARACTER -----------------------------
  template<typename Char__, typename Traits__, typename Alloc__>
  template<typename T>
  CCSTR_INLINE THIS_T& THIS_T::c(T v)
  {
    m_Composite.push_back(static_cast<_Char>(v));
    BuildCompositeChunk();
    return *this;
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<typename T>
  CCSTR_INLINE THIS_T& THIS_T::c(T v, size_t count)
  {
    m_Composite.reserve(m_Composite.size() + count);
    for(; count > 0; --count)
    {
      m_Composite.push_back(static_cast<_Char>(v));
    }
    BuildCompositeChunk();
    return *this;
  }

  // STRING -----------------------------
  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t MaxLen>
  CCSTR_INLINE THIS_T& THIS_T::s(const _Char* s)
  {
    if(s) m_Composite.append(s, MaxLen);
    BuildCompositeChunk();
    return *this;
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T& THIS_T::s(const _Char* s, size_t MaxLen)
  {
    if(s) m_Composite.append(s, MaxLen);
    BuildCompositeChunk();
    return *this;
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T& THIS_T::s(const _Char* s)
  {
    if(s) m_Composite.append(s);
    BuildCompositeChunk();
    return *this;
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<typename aChar>
  CCSTR_INLINE THIS_T& THIS_T::s(const aChar* foreign)
  {
    if(foreign)
    {
      _String native;
      StringCopy(native, foreign);
      m_Composite.append(native);
    }
    BuildCompositeChunk();
    return *this;
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t MaxLen, typename aChar>
  CCSTR_INLINE THIS_T& THIS_T::s(const aChar* foreign)
  {
    if(foreign)
    {
      _String native;
      StringCopyN(native, foreign, MaxLen);
      m_Composite.append(native);
    }
    BuildCompositeChunk();
    return *this;
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<typename aChar>
  CCSTR_INLINE THIS_T& THIS_T::s(const aChar* foreign, size_t MaxLen)
  {
    if(foreign)
    {
      _String native;
      StringCopyN(native, foreign, MaxLen);
      m_Composite.append(native);
    }
    BuildCompositeChunk();
    return *this;
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<typename aChar, typename aTraits, typename aAlloc>
  CCSTR_INLINE THIS_T& THIS_T::s(const std::basic_string<aChar, aTraits, aAlloc>& x)
  {
    return s(x.c_str());
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t MaxLen, typename aChar, typename aTraits, typename aAlloc>
  CCSTR_INLINE THIS_T& THIS_T::s(const std::basic_string<aChar, aTraits, aAlloc>& x)
  {
    return s<MaxLen>(x.c_str());
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<typename aChar, typename aTraits, typename aAlloc>
  CCSTR_INLINE THIS_T& THIS_T::s(const std::basic_string<aChar, aTraits, aAlloc>& x, size_t MaxLen)
  {
    return s(x.c_str(), MaxLen);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T& THIS_T::NewLine()
  {
    return s("\r\n");
  }

  // QUOTED STRINGS
  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t MaxLen>
  CCSTR_INLINE THIS_T& THIS_T::qs(const _Char* s)
  {
    if(MaxLen && s)
    {
      m_Composite.push_back('\"');
      if(MaxLen >= 3)
      {
        m_Composite.append(s, MaxLen - 2);
        m_Composite.push_back('\"');
      }
      else if(MaxLen == 2)
      {
        m_Composite.push_back('\"');
      }
    }
    BuildCompositeChunk();

    return *this;
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T& THIS_T::qs(const _Char* s, size_t MaxLen)
  {
    if(MaxLen && s)
    {
      m_Composite.push_back('\"');
      if(MaxLen >= 3)
      {
        m_Composite.append(s, MaxLen - 2);
        m_Composite.push_back('\"');
      }
      else if(MaxLen == 2)
      {
        m_Composite.push_back('\"');
      }
    }
    BuildCompositeChunk();
    return *this;
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T& THIS_T::qs(const _Char* s)
  {
    if(s)
    {
      m_Composite.push_back('\"');
      m_Composite.append(s);
      m_Composite.push_back('\"');
    }
    BuildCompositeChunk();
    return *this;
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<typename aChar>
  CCSTR_INLINE THIS_T& THIS_T::qs(const aChar* foreign)
  {
    if(foreign)
    {
      _String native;
      StringCopy(native, foreign);
      m_Composite.push_back('\"');
      m_Composite.append(native);
      m_Composite.push_back('\"');
    }
    BuildCompositeChunk();
    return *this;
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t MaxLen, typename aChar>
  CCSTR_INLINE THIS_T& THIS_T::qs(const aChar* foreign)
  {
    if(MaxLen && foreign)
    {
      m_Composite.push_back('\"');
      if(MaxLen >= 3)
      {
        _String native;
        StringCopyN(native, foreign, MaxLen);
        m_Composite.append(native.c_str(), MaxLen - 2);
        m_Composite.push_back('\"');
      }
      else if(MaxLen == 2)
      {
        m_Composite.push_back('\"');
      }
    }
    BuildCompositeChunk();
    return *this;
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<typename aChar>
  CCSTR_INLINE THIS_T& THIS_T::qs(const aChar* foreign, size_t MaxLen)
  {
    if(MaxLen && foreign)
    {
      m_Composite.push_back('\"');
      if(MaxLen >= 3)
      {
        _String native;
        StringCopyN(native, foreign, MaxLen);
        m_Composite.append(native.c_str(), MaxLen - 2);
        m_Composite.push_back('\"');
      }
      else if(MaxLen == 2)
      {
        m_Composite.push_back('\"');
      }
    }
    BuildCompositeChunk();
    return *this;
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<typename aChar, typename aTraits, typename aAlloc>
  CCSTR_INLINE THIS_T& THIS_T::qs(const std::basic_string<aChar, aTraits, aAlloc>& x)
  {
    return qs(x.c_str());
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t MaxLen, typename aChar, typename aTraits, typename aAlloc>
  CCSTR_INLINE THIS_T& THIS_T::qs(const std::basic_string<aChar, aTraits, aAlloc>& x)
  {
    return qs<MaxLen>(x.c_str());
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<typename aChar, typename aTraits, typename aAlloc>
  CCSTR_INLINE THIS_T& THIS_T::qs(const std::basic_string<aChar, aTraits, aAlloc>& x, size_t MaxLen)
  {
    return qs(x.c_str(), MaxLen);
  }

  // UNSIGNED LONG -----------------------------
  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t Base, size_t Width, Char__ PadChar>
  CCSTR_INLINE THIS_T& THIS_T::ul(unsigned long n)
  {
    const size_t BufferSize = _BufferSizeNeededInteger<Width, unsigned long>::Value;
    _Char buf[BufferSize];
    _Char* p = buf + BufferSize - 1;
    *p = 0;
    return s(_UnsignedNumberToString<Base, Width, PadChar>(p, n));
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t Base, size_t Width>
  CCSTR_INLINE THIS_T& THIS_T::ul(unsigned long n)
  {
    return ul<Base, Width, '0'>(n);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t Base>
  CCSTR_INLINE THIS_T& THIS_T::ul(unsigned long n)
  {
    return ul<Base, 0, 0>(n);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T& THIS_T::ul(unsigned long n)
  {
    return ul<10, 0, 0>(n);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T& THIS_T::ul(unsigned long n, size_t Base, size_t Width, _Char PadChar)
  {
    const size_t BufferSize = _RuntimeBufferSizeNeededInteger<unsigned long>(Width);
    _Char* buf = (_Char*)_alloca(BufferSize * sizeof(_Char));
    _Char* p = buf + BufferSize - 1;
    *p = 0;
    return s(_RuntimeUnsignedNumberToString<unsigned long>(p, n, Base, Width, PadChar));
  }

  // SIGNED LONG -----------------------------
  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t Base, size_t Width, Char__ PadChar, bool ForceShowSign>
  CCSTR_INLINE THIS_T& THIS_T::l(signed long n)
  {
    const size_t BufferSize = _BufferSizeNeededInteger<Width, signed long>::Value;
    _Char buf[BufferSize];
    _Char* p = buf + BufferSize - 1;
    *p = 0;
    return s(_SignedNumberToString<Base, Width, PadChar, ForceShowSign>(p, n));
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t Base, size_t Width, Char__ PadChar>
  CCSTR_INLINE THIS_T& THIS_T::l(signed long n)
  {
    return l<Base, Width, PadChar, false>(n);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t Base, size_t Width>
  CCSTR_INLINE THIS_T& THIS_T::l(signed long n)
  {
    return l<Base, Width, '0', false>(n);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t Base>
  CCSTR_INLINE THIS_T& THIS_T::l(signed long n)
  {
    return l<Base, 0, 0, false>(n);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T& THIS_T::l(signed long n)
  {
    return l<10, 0, 0, false>(n);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T& THIS_T::l(signed long n, size_t Base, size_t Width = 0, _Char PadChar = '0', bool ForceShowSign = false)
  {
    const size_t BufferSize = _RuntimeBufferSizeNeededInteger<unsigned long>(Width);
    _Char* buf = (_Char*)_alloca(BufferSize * sizeof(_Char));
    _Char* p = buf + BufferSize - 1;
    *p = 0;
    return s(_RuntimeSignedNumberToString(p, n, Base, Width, PadChar, ForceShowSign));
  }

  // FLOAT -----------------------------
  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t DecimalWidthMax, size_t IntegralWidthMin, Char__ PaddingChar, bool ForceSign, size_t Base>
  CCSTR_INLINE THIS_T& THIS_T::f(float val)
  {
    return _AppendFloat<SinglePrecisionFloat, Base, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign>(val);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t DecimalWidthMax, size_t IntegralWidthMin, Char__ PaddingChar, bool ForceSign>
  CCSTR_INLINE THIS_T& THIS_T::f(float val)
  {
    return f<DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign, 10>(val);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t DecimalWidthMax, size_t IntegralWidthMin, Char__ PaddingChar>
  CCSTR_INLINE THIS_T& THIS_T::f(float val)
  {
    return f<DecimalWidthMax, IntegralWidthMin, PaddingChar, false, 10>(val);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t DecimalWidthMax, size_t IntegralWidthMin>
  CCSTR_INLINE THIS_T& THIS_T::f(float val)
  {
    return f<DecimalWidthMax, IntegralWidthMin, '0', false, 10>(val);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t DecimalWidthMax>
  CCSTR_INLINE THIS_T& THIS_T::f(float val)
  {
    return f<DecimalWidthMax, 1, '0', false, 10>(val);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T& THIS_T::f(float val, size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign, size_t Base)
  {
    return _RuntimeAppendFloat<SinglePrecisionFloat>(val, Base, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign);
  }

  // DOUBLE -----------------------------
  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t DecimalWidthMax, size_t IntegralWidthMin, Char__ PaddingChar, bool ForceSign, size_t Base>
  CCSTR_INLINE THIS_T& THIS_T::d(double val)
  {
    return _AppendFloat<DoublePrecisionFloat, Base, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign>(val);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t DecimalWidthMax, size_t IntegralWidthMin, Char__ PaddingChar, bool ForceSign>
  CCSTR_INLINE THIS_T& THIS_T::d(double val)
  {
    return d<DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign, 10>(val);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t DecimalWidthMax, size_t IntegralWidthMin, Char__ PaddingChar>
  CCSTR_INLINE THIS_T& THIS_T::d(double val)
  {
    return d<DecimalWidthMax, IntegralWidthMin, PaddingChar, false, 10>(val);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t DecimalWidthMax, size_t IntegralWidthMin>
  CCSTR_INLINE THIS_T& THIS_T::d(double val)
  {
    return d<DecimalWidthMax, IntegralWidthMin, '0', false, 10>(val);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t DecimalWidthMax>
  CCSTR_INLINE THIS_T& THIS_T::d(double val)
  {
    return d<DecimalWidthMax, 1, '0', false, 10>(val);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T& THIS_T::d(double val, size_t DecimalWidthMax = 3, size_t IntegralWidthMin = 1, _Char PaddingChar = '0', bool ForceSign = false, size_t Base = 10)
  {
    return _RuntimeAppendFloat<DoublePrecisionFloat>(val, Base, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign);
  }

  // UNSIGNED INT 64 -----------------------------
  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t Base, size_t Width, Char__ PadChar>
  CCSTR_INLINE THIS_T& THIS_T::ui64(unsigned __int64 n)
  {
    const size_t BufferSize = _BufferSizeNeededInteger<Width, unsigned __int64>::Value;
    _Char buf[BufferSize];
    _Char* p = buf + BufferSize - 1;
    *p = 0;
    return s(_UnsignedNumberToString<Base, Width, PadChar>(p, n));
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t Base, size_t Width>
  CCSTR_INLINE THIS_T& THIS_T::ui64(unsigned __int64 n)
  {
    return ui64<Base, Width, '0'>(n);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t Base> 
  CCSTR_INLINE THIS_T& THIS_T::ui64(unsigned __int64 n)
  {
    return ui64<Base, 0, 0>(n);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T& THIS_T::ui64(unsigned __int64 n)
  {
    return ui64<10, 0, 0>(n);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T& THIS_T::ui64(unsigned __int64 n, size_t Base, size_t Width, _Char PadChar)
  {
    const size_t BufferSize = _RuntimeBufferSizeNeededInteger<unsigned __int64>(Width);
    _Char* buf = (_Char*)_alloca(BufferSize * sizeof(_Char));
    _Char* p = buf + BufferSize - 1;
    *p = 0;
    return s(_RuntimeUnsignedNumberToString<unsigned __int64>(p, n, Base, Width, PadChar));
  }

  // SIGNED INT 64 -----------------------------
  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t Base, size_t Width, Char__ PadChar, bool ForceShowSign>
  CCSTR_INLINE THIS_T& THIS_T::i64(signed __int64 n)
  {
    const size_t BufferSize = _BufferSizeNeededInteger<Width, unsigned __int64>::Value;
    _Char buf[BufferSize];
    _Char* p = buf + BufferSize - 1;
    *p = 0;
    return s(_SignedNumberToString<Base, Width, PadChar, ForceShowSign>(p, n));
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t Base, size_t Width, Char__ PadChar>
  CCSTR_INLINE THIS_T& THIS_T::i64(__int64 n)
  {
    return i64<Base, Width, PadChar, false>(n);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t Base, size_t Width>
  CCSTR_INLINE THIS_T& THIS_T::i64(__int64 n)
  {
    return i64<Base, Width, '0', false>(n);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t Base>
  CCSTR_INLINE THIS_T& THIS_T::i64(__int64 n)
  {
    return i64<Base, 0, 0, false>(n);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T& THIS_T::i64(__int64 n)
  {
    return i64<10, 0, 0, false>(n);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T& THIS_T::i64(signed __int64 n, size_t Base, size_t Width, _Char PadChar, bool ForceShowSign)
  {
    const size_t BufferSize = _RuntimeBufferSizeNeededInteger<unsigned __int64>(Width);
    _Char* buf = (_Char*)_alloca(BufferSize * sizeof(_Char));
    _Char* p = buf + BufferSize - 1;
    *p = 0;
    return s(_RuntimeSignedNumberToString<signed __int64>(p, n, Base, Width, PadChar, ForceShowSign));
  }

  // GETLASTERROR() -----------------------------
#ifdef CCSTR_WIN32
  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T& THIS_T::gle(int code)
  {
    _String str;
    FormatMessageGLE(str, code);
    return s(str);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T& THIS_T::gle()
  {
    return gle(GetLastError());
  }
#endif

  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE THIS_T& THIS_T::_RuntimeAppendZeroFloat(size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool)
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

  template<typename Char__, typename Traits__, typename Alloc__>
  template<typename FloatType>
  CCSTR_INLINE THIS_T& THIS_T::_RuntimeAppendNormalizedFloat(FloatType& _f, size_t Base, size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign)
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
  template<typename Char__, typename Traits__, typename Alloc__>
  template<typename FloatType>
  CCSTR_INLINE THIS_T& THIS_T::_RuntimeAppendFloat(const FloatType& _f, size_t Base, size_t DecimalWidthMax, size_t IntegralWidthMin, _Char PaddingChar, bool ForceSign)
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

  template<typename Char__, typename Traits__, typename Alloc__>
  template<typename FloatType, size_t Base, size_t DecimalWidthMax, size_t IntegralWidthMin, Char__ PaddingChar, bool ForceSign>
  CCSTR_INLINE THIS_T& THIS_T::_AppendFloat(const FloatType& _f)
  {
    return _RuntimeAppendFloat<FloatType>(_f, Base, DecimalWidthMax, IntegralWidthMin, PaddingChar, ForceSign);
  }

  template<typename Char__, typename Traits__, typename Alloc__>
  template<typename T>
  CCSTR_INLINE long THIS_T::_RuntimeBufferSizeNeededInteger(size_t Width)
  {
    return (sizeof(T) * 8) + 2 > (Width + 1) ? (sizeof(T) * 8) + 2 : (Width + 1);
  };

  // buf must point to a null terminator.  It is "pulled back" and the result is returned.
  // its simply faster to build the string in reverse order.
  template<typename Char__, typename Traits__, typename Alloc__>
  template<typename T>
  CCSTR_INLINE Char__* THIS_T::_RuntimeUnsignedNumberToString(_Char* buf, T num, size_t Base, size_t Width, _Char PaddingChar)
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

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t Base, size_t Width, Char__ PaddingChar, typename T>
  CCSTR_INLINE Char__* THIS_T::_UnsignedNumberToString(_Char* buf, T num)
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

  // same thing, but params can be set at runtime
  template<typename Char__, typename Traits__, typename Alloc__>
  template<typename T>
  CCSTR_INLINE Char__* THIS_T::_RuntimeSignedNumberToString(_Char* buf, T num, size_t Base, size_t Width, _Char PaddingChar, bool ForceSign)
  {
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

  template<typename Char__, typename Traits__, typename Alloc__>
  template<size_t Base, size_t Width, Char__ PaddingChar, bool ForceSign, typename T>
  CCSTR_INLINE Char__* THIS_T::_SignedNumberToString(_Char* buf, T num)
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
  template<typename Char__, typename Traits__, typename Alloc__>
  CCSTR_INLINE void THIS_T::BuildCompositeChunk()
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
          m_Composite.push_back('\r');
          m_Composite.push_back('\n');
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
}

#pragma warning(pop)
