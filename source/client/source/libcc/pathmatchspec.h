/*
  LibCC Release "March 9, 2007"
  PathMatchSpec Module
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


/*
    Criteria:   "c*cor?n"
    String:     "carl corcoran"

    Criteria list:
        Literal("c")
        Star
        Literal("cor")
        Question
        Literal("n")
        End-of-line

    Criteria Types:
        Literal - compares the exact given string to the current position, if it
                  matches, then continue with the next criteria list item.
        Star - advance the cursor until the next criteria list item matches.  If a
               next-file-part is found, no match.
        Question - advance the cursor by 1 blindly *so long as there's one char left*.
        EndOfLine - match only if there are no more chars
        Ellipse - match

    The string will match the criteria if the criteria is found at position 0.
*/

#pragma once


#include <string>
#include <vector>
#include <stack>
#include "StringUtil.h"
#include "Winapi.h"

namespace LibCC
{
  template<typename Char>
  class PathMatchSpecP4X
  {
    enum CriteriaEngine
    {
        CE_Star,
        CE_Question,
        CE_Literal,
        CE_Ellipses,
        CE_EndOfLine,
    };

    template<typename Char>
    class CriteriaEngineBase
    {
    public:
      typedef std::vector<CriteriaEngineBase<Char>*> CriteriaList;
      typedef typename CriteriaList::iterator CriteriaListIterator;
      typedef std::basic_string<Char> String;

      CriteriaEngineBase(CriteriaEngine type) :
        m_Type(type)
      {
      }

      virtual ~CriteriaEngineBase()
      {
      }

      // here, we will start at the current position and just perform the rest of the criteria.
      // if we encounter a path separator in our quest, no match.
      virtual bool Match(CriteriaList& cl, CriteriaListIterator itCriteria, const std::basic_string<Char>& s) = 0;
      CriteriaEngine m_Type;
    };

    template<typename Char>
    class QuestionType : public CriteriaEngineBase<Char>
    {
    public:
      QuestionType() :
        CriteriaEngineBase<Char>(CE_Question)
      {
      }
      bool Match(CriteriaList& cl, CriteriaListIterator itCriteria, const std::basic_string<Char>& s)
      {
        bool r = false;

        if(!s.empty())
        {
          if(!IsPathSeparator(s[0]))
          {
            std::basic_string<Char> sTheRest = s.substr(1);
            itCriteria++;
            CriteriaEngineBase<Char>* pType = *itCriteria;

            r = pType->Match(cl, itCriteria, sTheRest);
          }
        }

        return r;
      }
    };

    template<typename Char>
    class StarType : public CriteriaEngineBase<Char>
    {
    public:
      StarType() :
        CriteriaEngineBase<Char>(CE_Star)
      {
      }
      bool Match(CriteriaList& cl, CriteriaListIterator itCriteria, const std::basic_string<Char>& s)
      {
        bool r = false;
        itCriteria++;
        std::basic_string<Char>::const_iterator itsCriteria = s.begin();
        std::basic_string<Char>::size_type pos = 0;
        std::basic_string<Char> sTheRest;

        while(1)
        {
          // generate a string that represents the rest of the string
          sTheRest = s.substr(pos);

          if((*itCriteria)->Match(cl, itCriteria, sTheRest))
          {
            // this is the only way to cause a match.
            r = true;
            break;
          }

          // make sure we didn't encounter a path separator.
          if(IsPathSeparator(*itsCriteria)) break;

          if(itsCriteria == s.end()) break;// no match.

          itsCriteria ++;
          pos ++;
        }

        return r;
      }
    };

    template<typename Char>
    class EllipsesType : public CriteriaEngineBase<Char>
    {
    public:
      EllipsesType() :
        CriteriaEngineBase<Char>(CE_Ellipses)
      {
      }
      bool Match(CriteriaList& cl, CriteriaListIterator itCriteria, const std::basic_string<Char>& s)
      {
        bool r = false;
        itCriteria++;
        std::basic_string<Char>::const_iterator itsCriteria = s.begin();
        std::basic_string<Char>::size_type pos = 0;
        std::basic_string<Char> sTheRest;

        while(1)
        {
          // generate a string that represents the rest of the string
          sTheRest = s.substr(pos);

          if((*itCriteria)->Match(cl, itCriteria, sTheRest))
          {
            // this is the only way to cause a match.
            r = true;
            break;
          }

          if(itsCriteria == s.end()) break;// no match.

          itsCriteria ++;
          pos ++;
        }

        return r;
      }
    };

    template<typename Char>
    class EndOfLineType : public CriteriaEngineBase<Char>
    {
    public:
      EndOfLineType() :
        CriteriaEngineBase<Char>(CE_EndOfLine)
      {
      }
      bool Match(CriteriaList& cl, CriteriaListIterator itCriteria, const std::basic_string<Char>& s)
      {
          return (s.empty());
      }
    };

    template<typename Char>
    class LiteralType : public CriteriaEngineBase<Char>
    {
    public:
      LiteralType() :
        CriteriaEngineBase<Char>(CE_Literal)
      {
      }
      std::basic_string<Char> m_s;
      bool Match(CriteriaList& cl, CriteriaListIterator _itCriteria, const std::basic_string<Char>& s)
      {
          bool r = false;

          if(s.length() >= m_s.length())
          {
              bool bMatch = false;
              std::basic_string<Char>::const_iterator itCriteria;
              std::basic_string<Char>::const_iterator itComparison;
              std::basic_string<Char>::value_type cCriteria;
              std::basic_string<Char>::value_type cComparison;

              itCriteria = m_s.begin();
              itComparison = s.begin();

              while(1)
              {
                  if(itCriteria == m_s.end())
                  {
                      // If we hit the end of the criteria string that means everything matched
                      // so far; MATCH and BREAK
                      bMatch = true;
                      break;
                  }

                  if(itComparison == s.end())
                  {
                      // if we hit the end of the comparison string, no match.
                      break;
                  }

                  cCriteria = tolower(*itCriteria);
                  cComparison = tolower(*itComparison);

                  if(IsPathSeparator(cComparison))
                  {
                      // both must be path separators
                      if(!IsPathSeparator(cCriteria)) break;
                  }
                  else
                  {
                      // we hit a place where they are different.
                      if(cCriteria != cComparison) break;
                  }

                  itCriteria ++;
                  itComparison ++;
              }

              if(bMatch)
              {
                  _itCriteria++;
                  std::basic_string<Char> sTheRest;
                  sTheRest = s.substr(m_s.length());
                  r = (*_itCriteria)->Match(cl, _itCriteria, sTheRest);
              }
          }
          return r;
      }
    };
    typedef typename CriteriaEngineBase<Char>::CriteriaList CriteriaList;// list of criteria lists (for semicolon-separated dealies)
    typedef std::vector<CriteriaList> CriteriaListList;// list of criteria lists (for semicolon-separated dealies)

  public:
    PathMatchSpecP4X()
    {
    }

    ~PathMatchSpecP4X()
    {
    }

    typedef std::basic_string<Char> String;
    typedef typename std::basic_string<Char>::const_iterator StringConstIterator;

    void SetCriteria(const String& s)
    {
      ClearCriteria();

      // find the first segment (separated by semicolons)
      String::size_type nCurrentSemicolon = 0;
      String::size_type nPreviousSemicolon = 0;

      /*
          check out this criteria:   c:\\*code\\*.h;*.cpp
          here, I have to extract c:\\*code from the first segment
          and append that to every non-absolute segment thereafter.
      */
      String sDirectory;
      String sThisSegment;

      while(1)
      {
          if(nCurrentSemicolon)
          {
              nPreviousSemicolon = nCurrentSemicolon + 1;// only advance if it's actually on a semicolon, not if it's just at position 0
          }
          else
          {
              nPreviousSemicolon = nCurrentSemicolon;
          }

          nCurrentSemicolon = s.find(';', nPreviousSemicolon);// find the next semicolon.
          if(nCurrentSemicolon != String::npos)
          {
              sThisSegment = s.substr(nPreviousSemicolon, nCurrentSemicolon - nPreviousSemicolon);

              if(sDirectory.empty())
              {
                  if(PathIsAbsolute(sThisSegment))
                  {
                      // this thing is a full path and we don't have  adirectory; create it
                      sDirectory = PathRemoveFilename(sThisSegment);
                  }
              }
              else
              {
                  if(!PathIsAbsolute(sThisSegment))
                  {
                      sThisSegment = PathJoin(sDirectory, sThisSegment);
                  }
              }

              SetCriteria(GetNewCriteriaList(), sThisSegment);
          }
          else
          {
              // this is the last segment; no semicolon was found.
              sThisSegment = s.substr(nPreviousSemicolon);

              if(!sDirectory.empty())
              {
                  if(!PathIsAbsolute(sThisSegment))
                  {
                      sThisSegment = PathJoin(sDirectory, sThisSegment);
                  }
              }

              SetCriteria(GetNewCriteriaList(), sThisSegment);

              break;
          }
      }

      return;
    }

    bool Match(const String& s)
    {
      bool r = false;
      CriteriaListList::iterator it;
      //CriteriaList* pCriteriaItem = 0;

      for(it = m_CriteriaLists.begin(); it != m_CriteriaLists.end(); it++)
      {
        CriteriaList& item(*it);
        if(!item.empty())// empty criteria == no match.
        {
          typename CriteriaEngineBase<Char>::CriteriaListIterator itCriteria = item.begin();
          r = item.front()->Match(item, itCriteria, s);
        }

        if(r) break;// if we find a match, no reason to keep searching.
      }
      return r;
    }

    //void DumpCriteria();

    bool SpansSubDirectories()// convenience function so we don't always have to search sub directories.
    {
        bool r = false;
        CriteriaListList::iterator it;
        for(it=m_CriteriaLists.begin();it!=m_CriteriaLists.end();it++)
        {
            r = SpansSubDirectories(&(*it));
            if(r) break;
        }
        return r;
    }

    bool GetRootDir(String& s)// returns a path that can be searched for files to match teh criteria.
    {
        bool r = false;
        CriteriaList* pFirstList = &(m_CriteriaLists.front());
        CCriteriaTypeBase* pFirstCriteria = pFirstList->front();

        if(pFirstCriteria->m_Type == CT_Literal)
        {
            CLiteralType* pLiteral = (CLiteralType*)pFirstCriteria;
            s = PathRemoveFilename(pLiteral->m_s);
        }
        r = true;

        return r;
    }

    ///////////////////////////////////////////////////////////////////////////
    static bool IteratorPointsToEllipses(StringConstIterator& it, const String& s)
    {
        bool r = false;

        if(it != s.end())
        {
            if((*it) == '.')
            {
                // we have the first one.
                it++;
                if(it != s.end())
                {
                    if((*it) == '.')
                    {
                        // we have the second one.
                        it++;
                        if(it != s.end())
                        {
                            if((*it) == '.')
                            {
                                // we have the third one.
                                r = true;
                            }
                            else
                            {
                                it -= 2;
                            }
                        }
                        else
                        {
                            it -= 2;
                        }
                    }
                    else
                    {
                        it --;
                    }
                }
                else
                {
                    it --;
                }
            }
        }

        return r;
    }

  private:
    void ClearCriteria()
    {
      CriteriaListList::iterator itLists;
      CriteriaEngineBase<Char>::CriteriaListIterator it;
      CriteriaEngineBase<Char>::CriteriaList* pThisList = 0;

      for(itLists=m_CriteriaLists.begin();itLists!=m_CriteriaLists.end();itLists++)
      {
        pThisList = &(*itLists);
        for(it=pThisList->begin();it!=pThisList->end();it++)
        {
            delete (*it);
        }

        pThisList->clear();
      }

      m_CriteriaLists.clear();

      return;
    }

    CriteriaListList m_CriteriaLists;

    bool SpansSubDirectories(CriteriaList* pCriteria)
    {
      bool r = false;
      CriteriaList::reverse_iterator it;

      bool bHaveHitPathSep = false;

      for(it=pCriteria->rbegin();it!=pCriteria->rend();it++)
      {
          CCriteriaTypeBase* pBase = *it;
          switch(pBase->m_Type)
          {
          case CT_Ellipses:
              r = true;
              break;
          case CT_Star:
              if(bHaveHitPathSep) r = true;
              break;
          case CT_Question:
              if(bHaveHitPathSep) r = true;
              break;
          case CT_Literal:
              {
                  CLiteralType* plt = (CLiteralType*)pBase;
                  // search for path separator.
                  if(String::npos != plt->m_s.find_first_of("\\/"))
                  {
                      bHaveHitPathSep = true;
                  }
                  break;
              }
          }

          // if we satisfy the need-sub-dirs criteria, then exit immediately.
          if(r) break;
      }

      return r;
    }

    void SetCriteria(CriteriaList* pCriteria, const String& s)
    {
      // this basically "compiles" the string into our internal structure.
      String::const_iterator it = 0;
      String::value_type c = 0;

      for(it=s.begin();it!=s.end();it++)
      {
        c = *it;
        if(c == '*') { pCriteria->push_back(new StarType<Char>); }
        else if(c == '?') { pCriteria->push_back(new QuestionType<Char>); }
        else if(IteratorPointsToEllipses(it, s)) { pCriteria->push_back(new EllipsesType<Char>); }
        else
        {
          LiteralType<Char>* pCurrentLiteral = 0;
          // see if we need to push_Back a new literal engine.
          if(pCriteria->empty() || (pCriteria->back()->m_Type != CE_Literal))
          {
              pCurrentLiteral = new LiteralType<Char>;
              pCriteria->push_back(pCurrentLiteral);
          }
          else
          {
              CriteriaEngineBase<Char>* p = (pCriteria->back());
              pCurrentLiteral = (LiteralType<Char>*)p;
          }
          pCurrentLiteral->m_s.push_back(c);
        }
      }
      pCriteria->push_back(new EndOfLineType<Char>);
    }

    CriteriaList* GetNewCriteriaList()
    {
      m_CriteriaLists.push_back(CriteriaList());
      return &m_CriteriaLists.back();
    }
  };
  typedef PathMatchSpecP4X<char> PathMatchSpecP4A;
  typedef PathMatchSpecP4X<wchar_t> PathMatchSpecP4W;
  typedef PathMatchSpecP4X<TCHAR> PathMatchSpecP4;


  template<typename Char>
  inline bool PathMatchSpecSimple(const Char *wild, const Char *string)
  {
	  const Char *cp;
    const Char *mp;
  	
	  while ((*string) && (*wild != '*'))
    {
		  if ((*wild != *string) && (*wild != '?'))
      {
			  return 0;
		  }
		  wild++;
		  string++;
	  }
  		
	  while (*string)
    {
		  if (*wild == '*')
      {
			  if (!*++wild)
        {
				  return true;
			  }
			  mp = wild;
			  cp = string + 1;
		  }
      else if ((*wild == *string) || (*wild == '?')) 
      {
			  wild++;
			  string++;
		  }
      else 
      {
			  wild = mp;
			  string = cp++;
		  }
	  }
  		
	  while (*wild == '*')
    {
		  wild++;
	  }

	  return !*wild;
  }

  template<typename Char>
  class FileIteratorBaseX
  {
  public:
      virtual bool Reset(const std::basic_string<Char>& sDir) = 0;
      virtual bool Next(std::basic_string<Char>& sFullPath, DWORD* pdwAttributes) = 0;
  };


  // class for iterating through a single directory.
  template<typename Char>
  class FileIteratorX : public FileIteratorBaseX<Char>
  {
  public:
      FileIteratorX() :
        m_hFind(0)
      {
      }

      ~FileIteratorX()
      {
        _Free();
      }

      bool Reset(const std::basic_string<Char>& sDir)
      {
        bool r = false;

        _Free();

        std::string sQuery;
        sQuery = PathJoin(sDir, std::basic_string<Char>("*"));

        m_BaseDir = sDir;
        m_hFind = FindFirstFile(sQuery.c_str(), &m_fd);

        if(IsBadHandle(m_hFind))
        {
          // reset back to invalid.
          m_hFind = 0;
          m_BaseDir.clear();
        }
        else
        {
          r = true;
        }

        return r;
      }

      bool Next(std::basic_string<Char>& sFullPath, DWORD* pdwAttributes = 0)
      {
        bool r = false;
        if(IsValidHandle(m_hFind))
        {
          while(1)
          {
            if(IsUsableFileName(&m_fd))
            {
              // use it!
              sFullPath = PathJoin(m_BaseDir, std::basic_string<Char>(m_fd.cFileName));
              if(pdwAttributes) *pdwAttributes = m_fd.dwFileAttributes;
              r = true;

              // advance to the next one so we're queued up next call.
              if(0 == FindNextFile(m_hFind, &m_fd))
              {
                FindClose(m_hFind);
                m_hFind = 0;
              }
              break;
            }

            // otherwise, proceed to the next one.
            if(0 == FindNextFile(m_hFind, &m_fd))
            {
              // failure; make sure we clean everything up and return false.
              FindClose(m_hFind);
              m_hFind = 0;
              break;
            }
          }
        }
        return r;
      }

  private:
      bool IsUsableFileName(const WIN32_FIND_DATA* pfd)
      {
        bool r = true;
        if(pfd->cFileName[0])
        {
          if(pfd->cFileName[0] == '.')
          {
            if(pfd->cFileName[1] == 0)
            {
              // it equals "."
              r = false;
            }
            else
            {
              if(pfd->cFileName[1] == '.')
              {
                if(pfd->cFileName[2] == 0) r = false;// it equals ".."
              }
            }
          }
        }
        return r;
      }

      bool IsDirectory(const WIN32_FIND_DATA* pfd);

      void _Free()
      {
        if(IsValidHandle(m_hFind)) FindClose(m_hFind);
        m_hFind = 0;
      }

      std::basic_string<Char> m_BaseDir;// directory for this find
      HANDLE m_hFind;
      WIN32_FIND_DATA m_fd;
  };

  template<typename Char>
  class RecursiveFileIteratorX : public FileIteratorBaseX<Char>
  {
  public:
      ~RecursiveFileIteratorX()
      {
        _Free();
      }

      bool Reset(const std::basic_string<Char>& sDir)
      {
        bool r = false;
        m_stack.push(CFileIterator());
        FileIteratorX<Char>& it = m_stack.top();
        if(it.Reset(sDir))
        {
          r = true;
        }
        else
        {
          m_stack.pop();
        }

        return r;
      }

      bool Next(std::basic_string<Char>& sFullPath, DWORD* pdwAttributes = 0)
      {
        bool r = false;
        DWORD dwAttr = 0;

        while(m_stack.size())
        {
          FileIteratorX<Char>& it = m_stack.top();
          if(it.Next(sFullPath, &dwAttr))
          {
            if(pdwAttributes) *pdwAttributes = dwAttr;
            if(dwAttr & FILE_ATTRIBUTE_DIRECTORY)
            {
              // push an enum to this thing.
              m_stack.push(FileIteratorX<Char>());
              FileIteratorX<Char>& it = m_stack.top();
              it.Reset(sFullPath);
            }

            r = true;
            break;
          }
          else
          {
            m_stack.pop();
          }
        }

        return r;
      }

  private:
      std::stack<FileIteratorX<Char> > m_stack;
      void _Free()
      {
        while(m_stack.size())
        {
          m_stack.pop();
        }
      }
  };


}