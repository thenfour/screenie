/*
  Last updated Jan 9, 2005

  (c) 2004-2005 Carl Corcoran, carl@ript.net
  http://carl.ript.net/stringformat/
  http://carl.ript.net/wp
  http://mantis.winprog.org/
  http://svn.winprog.org/personal/carlc/stringformat

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

#include "libccoptions.h"
#include <string>

namespace LibCC
{
  char DigitToChar(unsigned char d);

  template<typename Char>
    inline size_t StringLength(const Char* in);

  // Copy
  template<typename LChar, typename Char>
    inline void StringCopy(std::basic_string<LChar>& out, const Char* in);
  template<typename String, typename Char>
    inline void StringCopyN(String& out, const Char* in, size_t maxlen);
  template<typename LChar, typename RChar>
    inline void StringCopyN(LChar* out, const RChar* in, size_t maxlen);
  template<typename DestString, typename SourceChar>
    inline void StringCopy(DestString& out, const std::basic_string<SourceChar>& in);
  template<typename DestString, typename SourceChar>
    inline void StringCopyN(DestString& out, const std::basic_string<SourceChar>& in, size_t maxlen);
  template<typename DestChar, typename SourceChar>
    inline std::basic_string<DestChar> StringCopy(const std::basic_string<SourceChar>& s);

  // Split
  template<typename Char, typename OutIt>
    inline void StringSplit(const std::basic_string<Char>& s, const std::basic_string<Char>& sep, OutIt dest);
  template<typename Char, typename OutIt>
    inline void StringSplit(const std::basic_string<Char>& s, const Char* sep, OutIt dest);
  template<typename Char, typename OutIt>
    inline void StringSplit(const std::basic_string<Char>& s, Char sep, OutIt dest);

  // Join
  template<typename InIt, typename Char>
    inline std::basic_string<Char> StringJoin(InIt start, InIt end, const Char* sep);
  template<typename InIt, typename Char>
    inline std::basic_string<Char> StringJoin(InIt start, InIt end, const std::basic_string<Char>& sep);
  template<typename LChar, typename RChar>
    inline void StringAppend(std::basic_string<LChar>& sep, const RChar* rhs);

  // Trim
  template<typename Char>
    inline std::basic_string<Char> StringTrim(const std::basic_string<Char>& s, const std::basic_string<Char>& chars);
  template<typename Char>
    inline std::basic_string<Char> StringTrim(const std::basic_string<Char>& s, const Char* chars);
  template<typename Char>
    inline std::basic_string<Char> StringTrim(const Char* s, const Char* chars);

  // Compare
  template<typename LChar, typename RChar>
    inline bool StringEquals(const std::basic_string<LChar>& lhs, const std::basic_string<RChar>& rhs);
  template<typename LChar, typename RChar>
    inline bool StringEquals(const std::basic_string<LChar>& lhs, const RChar* rhs);
  template<typename LChar, typename RChar>
    inline bool StringEquals(const LChar* lhs, const std::basic_string<RChar>& rhs);
  template<typename LChar, typename RChar>
    inline bool StringEquals(const LChar* lhs, const RChar* rhs);

  template<typename LChar, typename RChar>
    inline bool StringEqualsI(const std::basic_string<LChar>& lhs, const std::basic_string<RChar>& rhs);
  template<typename LChar, typename RChar>
    inline bool StringEqualsI(const std::basic_string<LChar>& lhs, const RChar* rhs);
  template<typename LChar, typename RChar>
    inline bool StringEqualsI(const LChar* lhs, const std::basic_string<RChar>& rhs);
  template<typename LChar, typename RChar>
    inline bool StringEqualsI(const LChar* lhs, const RChar* rhs);

  // Replace
  template<typename Char, typename SearchChar, typename ReplaceChar>
    inline std::basic_string<Char> StringReplace(const Char* src, const SearchChar* search, const ReplaceChar* replace);

  // Search
  template<typename Char, typename SearchChar>
    inline std::string::size_type StringFindFirstOf(const std::basic_string<Char>& s, const std::basic_string<SearchChar>& chars);
  template<typename Char, typename SearchChar>
    inline std::string::size_type StringFindFirstOf(const std::basic_string<Char>& s, const SearchChar* chars);

  template<typename Char, typename SearchChar>
    inline std::string::size_type StringFindI(const std::basic_string<Char>& s, const std::basic_string<SearchChar>& chars);
  template<typename Char, typename SearchChar>
    inline std::string::size_type StringFindI(const std::basic_string<Char>& s, const SearchChar* chars);
  template<typename Char, typename SearchChar>
    inline std::string::size_type StringFindI(const Char* s, const SearchChar* chars);

  template<typename Char, typename SearchChar>
    inline std::string::size_type StringFindLastOf(const std::basic_string<Char>& s, const std::basic_string<SearchChar>& chars);
  template<typename Char, typename SearchChar>
    inline std::string::size_type StringFindLastOf(const std::basic_string<Char>& s, const SearchChar* chars);

  template<typename Char, typename CharSearch>
    inline bool StringContains(const Char* s, CharSearch x);
  template<typename Char, typename CharSearch>
    inline bool StringContainsI(const std::basic_string<Char>& s, const CharSearch* x);

  template<typename Char, typename CharSearch>
    inline bool StringStartsWith(const Char* s, const CharSearch* search);
  template<typename Char, typename CharSearch>
    inline bool StringStartsWithI(const Char* s, const CharSearch* search);

  // transform
  template<typename Char>
    inline std::basic_string<Char> StringToUpper(const std::basic_string<Char> &s);
  template<typename Char>
    inline std::basic_string<Char> StringToLower(const std::basic_string<Char> &s);

}

#include "StringBaseImpl.h"

