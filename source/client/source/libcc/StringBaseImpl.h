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

// This is an inline h file for implementation.  Do not #include it.

namespace LibCC
{
  // String Conversion / Transformation Implementation -----------------------------------------------------------------------------------
  inline char DigitToChar(unsigned char d)
  {
    static const char Digits [] = "0123456789abcdefghijklmnopqrstuvwxyz";
    return d < (sizeof(Digits) / sizeof(char)) ? Digits[d] : 0;
  }

  template<typename Char>
  size_t StringLength(const Char* in)
  {
    size_t r(0);
    while(*in)
    {
      ++ r;
      ++ in;
    }
    return r;
  }

  template<typename LChar, typename Char>
  inline void StringCopy(std::basic_string<LChar>& out, const Char* in)
  {
    out.reserve(StringLength(in));
    while(*in)
    {
      out.push_back(static_cast<LChar>(*in));
      ++ in;
    }
  }

  template<typename LChar, typename RChar>
  inline void StringCopyN(LChar* out, const RChar* in, size_t maxlen)
  {
    while(*in && maxlen > 1)
    {
      *out = static_cast<LChar>(*in);
      ++ out;
      ++ in;
      maxlen --;
    }
    *out = 0;// null-terminate
  }

  template<typename String, typename Char>
  inline void StringCopyN(String& out, const Char* in, size_t maxlen)
  {
    out.reserve(StringLength(in));
    while(*in && maxlen --)
    {
      out.push_back(static_cast<typename String::value_type>(*in));
      ++ in;
    }
  }

  template<typename DestString, typename SourceChar>
  inline void StringCopy(DestString& out, const std::basic_string<SourceChar>& in)
  {
    out.reserve(in.size());
    std::basic_string<SourceChar>::const_iterator it;
    for(it = in.begin(); it != in.end(); ++ it)
    {
      out.push_back(static_cast<typename DestString::value_type>(*it));
    }
  }

  template<typename DestString, typename SourceChar>
  inline void StringCopyN(DestString& out, const std::basic_string<SourceChar>& in, size_t maxlen)
  {
    out.reserve(in.size());
    std::basic_string<SourceChar>::const_iterator it;
    for(it = in.begin(); (it != in.end()) && (maxlen --); ++ it)
    {
      out.push_back(static_cast<typename DestString::value_type>(*it));
    }
  }

  // std::vector<std::string> Parts;
  // StringSplit(s, ".", std::back_inserter(Parts));
  template<typename Char, class OutIt>
  inline void StringSplit(const std::basic_string<Char>& s, const std::basic_string<Char>& sep, OutIt dest)
  {
    std::basic_string<Char>::size_type left = s.find_first_not_of(sep);
    std::basic_string<Char>::size_type right = s.find_first_of(sep, left);
    while( left < right )
    {
      *dest = s.substr(left, right - left);
      ++dest;
      left = s.find_first_not_of(sep, right);
      right = s.find_first_of(sep, left);
    }
  }

  template<typename Char, typename OutIt>
    inline void StringSplit(const std::basic_string<Char>& s, const Char* sep, OutIt dest)
  {
    StringSplit(s, std::basic_string<Char>(sep), dest);
  }
  template<typename Char, typename OutIt>
    inline void StringSplit(const std::basic_string<Char>& s, Char sep, OutIt dest)
  {
    Char x[2] = { sep };
    StringSplit(s, x, dest);
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
    return StringJoin<InIt, Char>(start, end, std::basic_string<Char>(sep));
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
    return std::string(s, left, 1 + right - left);
  }

  template<typename Char>
  inline std::basic_string<Char> StringTrim(const std::basic_string<Char>& s, const Char* chars)
  {
    return StringTrim(s, std::basic_string<Char>(chars));
  }
  template<typename Char>
    inline std::basic_string<Char> StringTrim(const Char* s, const Char* chars)
  {
    return StringTrim(std::basic_string<Char>(s), std::basic_string<Char>(chars));
  }

  template<typename LChar, typename RChar>
  inline bool StringEquals(const std::basic_string<LChar>& lhs, const std::basic_string<RChar>& rhs)
  {
    return StringEquals(lhs.c_str(), rhs.c_str());
  }

  template<typename LChar, typename RChar>
  inline bool StringEquals(const LChar* lhs, const std::basic_string<RChar>& rhs)
  {
    return StringEquals(lhs, rhs.c_str());
  }

  template<typename LChar, typename RChar>
  inline bool StringEquals(const std::basic_string<LChar>& lhs, const RChar* rhs)
  {
    return StringEquals(lhs.c_str(), rhs);
  }

  template<typename LChar, typename RChar>
  inline bool StringEquals(const LChar* lhs, const RChar* rhs)
  {
    while(*lhs == *rhs)
    {
      if(*lhs == 0)
      {
        // they are the same all the way to the null term.
        return true;
      }
      lhs ++;
      rhs ++;
    }
    return false;
  }

  template<typename LChar, typename RChar>
  inline bool StringEqualsI(const std::basic_string<LChar>& lhs, const std::basic_string<RChar>& rhs)
  {
    return StringEqualsI(lhs.c_str(), rhs.c_str());
  }

  template<typename LChar, typename RChar>
  inline bool StringEqualsI(const LChar* lhs, const std::basic_string<RChar>& rhs)
  {
    return StringEqualsI(lhs, rhs.c_str());
  }

  template<typename LChar, typename RChar>
  inline bool StringEqualsI(const std::basic_string<LChar>& lhs, const RChar* rhs)
  {
    return StringEqualsI(lhs.c_str(), rhs);
  }

  template<typename LChar, typename RChar>
  inline bool StringEqualsI(const LChar* lhs, const RChar* rhs)
  {
    while(tolower(*lhs) == tolower(*rhs))
    {
      if(*lhs == 0)
      {
        // they are the same all the way to the null term.
        return true;
      }
      lhs ++;
      rhs ++;
    }
    return false;
  }

  template<typename Char, typename CharSearch>
  inline bool StringContains(const Char* s, CharSearch x)
  {
    while(*s)
    {
      if(*s == x) return true;
      s ++;
    }
    return false;
  }

  template<typename Char, typename SearchChar>
  inline std::string::size_type StringFindFirstOf(const std::basic_string<Char>& s, const SearchChar* chars)
  {
    const Char* p = s.c_str();
    while(*p)
    {
      if(StringContains(chars, *p))
      {
        return p - s.c_str();
      }
      p ++;
    }
    return std::string::npos;
  }

  template<typename Char, typename SearchChar>
  inline std::string::size_type StringFindFirstOf(const std::basic_string<Char>& s, const std::basic_string<SearchChar>& chars)
  {
    return StringFindFirstOf(s, chars.c_str());
  }

  template<typename Char, typename SearchChar>
  inline std::string::size_type StringFindLastOf(const std::basic_string<Char>& s, const SearchChar* chars)
  {
    if(!s.empty())
    {
      const Char* p = s.c_str() + s.size() - 1;
      do
      {
        if(StringContains(chars, *p))
        {
          return p - s.c_str();
        }
        p --;
      }
      while(p != s.c_str());
    }
    return std::string::npos;
  }

  template<typename Char, typename SearchChar>
  inline std::string::size_type StringFindLastOf(const std::basic_string<Char>& s, const std::basic_string<SearchChar>& chars)
  {
    return StringFindLastOf(s, chars.c_str());
  }


  template<typename Char, typename SearchChar>
  inline std::string::size_type StringFindI(const std::basic_string<Char>& s, const std::basic_string<SearchChar>& rhs)
  {
    return StringFindI(s, rhs.c_str());
  }

  template<typename Char, typename SearchChar>
  inline std::string::size_type StringFindI(const Char* s, const SearchChar* chars)
  {
    const Char* i = s;
    while(*i)
    {
      if(StringStartsWithI(i, chars))
      {
        return i - s;
      }
      i ++;
    }
    return std::string::npos;
  }

  template<typename Char, typename SearchChar>
  inline std::string::size_type StringFindI(const std::basic_string<Char>& s, const SearchChar* rhs)
  {
    return StringFindI(s.c_str(), rhs);
  }

  template<typename Char, typename CharSearch>
  inline bool StringContainsI(const std::basic_string<Char>& s, const CharSearch* x)
  {
    return StringFindI(s.c_str(), x) != std::string::npos;
  }

  template<typename Char>
  inline std::basic_string<Char> StringToUpper(const std::basic_string<Char> &s)
  {
    std::basic_string<Char> r;
    r.reserve(s.size());
    std::basic_string<Char>::const_iterator it;
    for(it = s.begin(); it != s.end(); ++ it)
    {
      r.push_back(toupper(*it));
    }
    return r;
  }

  template<typename Char>
  inline std::basic_string<Char> StringToLower(const std::basic_string<Char> &s)
  {
    std::basic_string<Char> r;
    r.reserve(s.size());
    std::basic_string<Char>::const_iterator it;
    for(it = s.begin(); it != s.end(); ++ it)
    {
      r.push_back(tolower(*it));
    }
    return r;
  }

  template<typename Char, typename CharSearch>
  inline bool StringStartsWith(const Char* s, const CharSearch* search)
  {
    while(*search)
    {
      if(*s != *search) return false;
      search ++;
      s ++;
    }
    return true;
  }

  template<typename Char, typename CharSearch>
  inline bool StringStartsWithI(const Char* s, const CharSearch* search)
  {
    while(*search)
    {
      if(tolower(*s) != tolower(*search)) return false;
      search ++;
      s ++;
    }
    return true;
  }

  template<typename LChar, typename RChar>
  inline void StringAppend(std::basic_string<LChar>& str, const RChar* rhs)
  {
    Blob<LChar> temp;
    temp.Alloc(StringLength(rhs) + 1);
    StringCopyN(temp.GetBuffer(), rhs, temp.Size() - 1);
    str.append(temp.GetBuffer());
    return;
  }

  template<typename Char, typename SearchChar, typename ReplaceChar>
  inline std::basic_string<Char> StringReplace(const Char* src, const SearchChar* search, const ReplaceChar* replace)
  {
    typedef std::basic_string<Char> _String;
    _String r;
    const Char* p = src;
    size_t searchLen = StringLength(search);
    size_t replaceLen = StringLength(replace);

    r.reserve(StringLength(src));
    while(*p)
    {
      if(StringStartsWith(p, search))
      {
        StringAppend(r, replace);
        p += searchLen;
      }
      else
      {
        p ++;
      }
    }
    return r;
  }

  template<typename DestChar, typename SourceChar>
  inline std::basic_string<DestChar> StringCopy(const std::basic_string<SourceChar>& s)
  {
    std::basic_string<DestChar> ret;
    ret.reserve(s.length());
    for(std::basic_string<SourceChar>::const_iterator it = s.begin(); it != s.end(); ++ it)
    {
      ret.push_back(static_cast<DestChar>(*it));
    }
    return ret;
  }

}

