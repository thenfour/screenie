

#pragma once

// auto-class for LoadLibrary() / FreeLibrary()
//class LibraryHandle
//{
//public:
//  LibraryHandle() : h(0) { }
//  LibraryHandle(HMODULE h_) : h(h_) { }
//  LibraryHandle& operator =(HMODULE h_) { Free(); h = h_; } 
//  void Free()
//  {
//    if(h)
//    {
//      FreeLibrary(h);
//    }
//  }
//  HMODULE h;
//};

// splits a string at the first equal sign.
Result ParseNameValueString(const string& in, string& name, string& value)
{
  string::size_type nEq = in.find('=');
  if(nEq == string::npos)
  {
    return Result(false, "Invalid value... no equal sign was found.");
  }
  name = in.substr(0, nEq);
  value = in.c_str() + nEq + 1;// i'm not sure you can substr() at the null-term, so i'll do it this way where it's guaranteed to be safe
  return Result(true);
}


// decimal value.  make sure there's no crap though.
template<int base>
inline Result ParseXBaseString(const string& s, DWORD& out)
{
  char* pEnd;
  out = strtoul(s.c_str(), &pEnd, base);
  if(*pEnd != 0)
  {
    return Result(E_FAIL, Format("Error parsing base % string %.  Invalid characters were found.").i(base).qs(s));
  }
  return Result(S_OK);
}

// either 0xnnnnnn or nnnnn
inline Result ParseNonBraketedString(const string& s, DWORD& out)
{
  if(s.length() < 3)// shortest hex value is 0x0
  {
    return ParseXBaseString<10>(s, out);
  }
  // is it decimal or hex?
  if(s[0] == '0' && s[1] == 'x')
  {
    return ParseXBaseString<16>(s, out);
  }
  return ParseXBaseString<10>(s, out);
}

bool FitsInAWord(DWORD dw)
{
  return dw < 0x10000;
}

bool FitsInAByte(DWORD dw)
{
  return dw < 0x100;
}

template<char closingBrace, int base>
inline Result ParseBraketedXBaseString(const string& s, DWORD& out)
{
  if((*s.rbegin()) != closingBrace)
  {
    return Result(false, Format("Closing '%' was not found.").c(closingBrace));
  }
  string inner = s.substr(1, s.length() - 2);// extract inner values
  vector<string> valueStrings;
  LibCC::StringSplit(inner, ' ', std::back_inserter(valueStrings));
  vector<DWORD> values;
  if(valueStrings.size() != 2 && valueStrings.size() != 4 && valueStrings.size() != 1)
  {
    return Result(false, Format("Invalid number of values specified (expected: 1, 2, or 4. actual: %).").st(valueStrings.size()));
  }
  for(vector<string>::iterator it = valueStrings.begin(); it != valueStrings.end(); ++ it)
  {
    Result res;
    DWORD temp;
    if(!(res = ParseXBaseString<base>(*it, temp)))
    {
      return res.Prepend(Format("Error parsing bracketed base % string; ").i(base));
    }
    values.push_back(temp);
  }
  switch(valueStrings.size())
  {
  case 1:
    // [0]
    out = values[0];
    break;
  case 2:
    // [0 1] - each part must fit in a word
    if(!FitsInAWord(values[0]))
    {
      return Result(E_FAIL, "First value is too large");
    }
    if(!FitsInAWord(values[1]))
    {
      return Result(E_FAIL, "Second value is too large");
    }
    out = (values[0] << 16) | values[1];
    break;
  case 4:
    // [0 1 2 3] - each part must fit in a byte
    if(!FitsInAByte(values[0]))
    {
      return Result(E_FAIL, "First value is too large");
    }
    if(!FitsInAByte(values[1]))
    {
      return Result(E_FAIL, "Second value is too large");
    }
    if(!FitsInAByte(values[2]))
    {
      return Result(E_FAIL, "Third value is too large");
    }
    if(!FitsInAByte(values[3]))
    {
      return Result(E_FAIL, "Fourth value is too large");
    }
    out = (values[0] << 24) | (values[1] << 16) | (values[2] << 8) | values[3];
    break;
  }
  return Result(S_OK);
}

inline Result ParseDWORDString(const string& s, DWORD& out)
{
  Result res;
  if(s.length() == 0)
  {
    return Result(false, "The string is empty.");
  }
  if(s.length() < 3)// shortest braketed value is [0]
  {
    return ParseNonBraketedString(s, out);
  }
  // is it braketed or not?
  if(s[0] == '[')
  {
    return ParseBraketedXBaseString<']', 16>(s, out);
  }
  if(s[0] == '{')
  {
    return ParseBraketedXBaseString<'}', 10>(s, out);
  }
  return ParseNonBraketedString(s, out);
}


