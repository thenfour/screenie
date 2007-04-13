/*
  LibCC Release "March 9, 2007"
  Blob Module
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
  Major Revisions:
    January 26, 2005 carlc
    - changed long types to size_t
    - fixed alloc bug (it was returning false if we had enough allocated already)
    - changed default lockable to false.

    May 22, 2005 carlc
    - Removed ability to lock.  this will break existing code that uses Blob, but
      there's just no situation where this class needs to be locked.  CString uses
      locking because it needs to know when it can replace the buffer pointer,
      because it can reallocate on almost any function call.  Here, the only time
      that will happen is the very clear & obvious "Alloc" function.
    - Combined ReAlloc() and Alloc().... why on earth would you use one over the
      other?

  Description:
    very FAST wrapper for quick malloc/free.  Used for quick memory buffers.
    Now, the only difference between this and a smart ptr is that here, we have
    1) static buffer support, so you can use a small buffer without allocating anything
      on the heap
    2) realloc support.

	  Instead of:

	  wchar_t* x = new wchar_t[MAX_PATH];
	  GetCurrentDirectoryW(x, MAX_PATH);
	  delete [] x;

	  Use:

	  BlobW x;
	  x.Alloc(MAX_PATH);
	  GetCurrentDirectoryW(x.GetWritableBuffer(), MAX_PATH);
*/


#pragma once

#include <windows.h>

#pragma warning(push)

/*
  Disable "conditional expression is constant".  This code has conditions
  on integral template params, which are by design constant.  This is a lame
  warning.
*/
#pragma warning(disable:4127)

namespace LibCC
{
  template<bool StaticBufferSupport = true, size_t StaticBufferSize = 100>// in reality, you should never change these, or else Blob types are really incompatible.
  class BlobTraits
  {
  public:
    static const bool _StaticBufferSupport = StaticBufferSupport;
    static const size_t _StaticBufferSize = StaticBufferSize;

    // return the new size, in elements
    static size_t GetNewSize(size_t current_size, size_t requested_size)
    {
      if(!current_size)
      {
        return requested_size;
      }

      // same as (current_size * 1.5)
      while(current_size < requested_size)
      {
        current_size += 1 + (current_size >> 1);
      }
      return current_size;
    }
  };

  // manages a simple memory blob.
  // an unallocated state will have m_p = 0 and m_size = 0
  //
  // if the class is "not lockable" that means in exchange for removing all the "lockable" checks (for performance), this class
  // will allow direct access to the buffer via GetLockedBuffer()).
  template<typename Tel = BYTE, typename Ttraits = BlobTraits<> >
  class Blob
  {
  private:
		template<typename T>
    void operator =(const T&)
		{ // no assignment available
		}
    Blob(const Blob<Tel, Ttraits>&)
    { // no copy construction available
    }

  public:

    typedef Tel _El;
    typedef Ttraits _Traits;
    static const bool _StaticBufferSupport = _Traits::_StaticBufferSupport;
    static const size_t _StaticBufferSize = _Traits::_StaticBufferSize;

    Blob() :
      m_p(_StaticBufferSupport ? m_StaticBuffer : 0),
      m_allocatedSize(_StaticBufferSupport ? _StaticBufferSize : 0),
      m_reportedSize(0)
    {
    }

    Blob(size_t size) :
      m_p(_StaticBufferSupport ? m_StaticBuffer : 0),
      m_allocatedSize(_StaticBufferSupport ? _StaticBufferSize : 0),
      m_reportedSize(0)
    {
			Alloc(size);
    }

    ~Blob()
    {
      Free();
    }

    bool Assign(const Blob<_El, _Traits>& rhs)
    {
      bool r = false;
      if(Alloc(rhs.Size()))
      {
        memcpy(GetBuffer(), rhs.GetBuffer(), rhs.Size() * sizeof(_El));
        r = true;
      }
      return r;
    }

    size_t Size() const
    {
      return m_reportedSize;
    }

    size_t AllocatedSize() const
    {
      return m_allocatedSize;
    }

		void Fill(const Tel& val)
		{
			for(size_t i = 0; i < m_reportedSize; ++ i)
			{
				m_p[i] = val;
			}
		}

    // these are just to break up some if() statements.
    inline bool CurrentlyUsingStaticBuffer() const
    {
      // is static buffer support enabled, and are we using the static buffer right now?
      return _StaticBufferSupport && (m_p == m_StaticBuffer);
    }
    inline bool CompletelyUnallocated() const
    {
      // is static buffer support disabled, and do we have NOTHING allocated now?
      return (!_StaticBufferSupport) && (m_p == 0);
    }

    // frees memory if its not locked.
    bool Free()
    {
      // why do i check both TLockable and m_locked?  Because if the compiler is smart enough,
      // it will totally eliminate the comparison if TLockable is always false (in the template param).
      // msvc 7.1 has been verified to successfully remove the compares.
      if(_StaticBufferSupport)
      {
        if(m_StaticBuffer != m_p)
        {
#ifdef DEBUG
					free(m_p);
#else
          HeapFree(GetProcessHeap(), 0, m_p);
#endif
          m_p = m_StaticBuffer;
          m_allocatedSize = _StaticBufferSize;
        }
        m_reportedSize = 0;
      }
      else
      {
        if(m_p)
        {
#ifdef DEBUG
					free(m_p);
#else
          HeapFree(GetProcessHeap(), 0, m_p);
#endif
          m_p = 0;
          m_allocatedSize = 0;
        }
        m_reportedSize = 0;
      }
      return true;
    }

    bool Alloc(size_t n)
    {
      bool r = false;
      if(m_allocatedSize >= n)
      {
        // no need to allocate; the big question is... what happens if you request smaller than what was already allocated? it will show the lower value.
        m_reportedSize = n;
        r = true;
      }
      else
      {
        // we definitely need to allocate now.
        size_t nNewSize = Ttraits::GetNewSize(m_allocatedSize, n);
        Tel* pNew;

        if(CurrentlyUsingStaticBuffer() || CompletelyUnallocated())
        {
          // allocate for the first time.
#ifdef DEBUG
					pNew = static_cast<Tel*>(malloc(sizeof(Tel) * nNewSize));
#else
          pNew = static_cast<Tel*>(HeapAlloc(GetProcessHeap(), 0, sizeof(Tel) * nNewSize));
#endif
          if(pNew)
          {
            if(CurrentlyUsingStaticBuffer())
            {
              // copy the contents of the static buffer into the new heap memory.
              CopyMemory(pNew, m_p, _StaticBufferSize);
            }

            m_p = pNew;
            m_allocatedSize = nNewSize;
            m_reportedSize = n;

            r = true;
          }
        }
        else
        {
          // realloc, because we already have a heap buffer.
#ifdef DEBUG
          pNew = static_cast<Tel*>(realloc(m_p, sizeof(Tel) * nNewSize));
#else
          pNew = static_cast<Tel*>(HeapReAlloc(GetProcessHeap(), 0, m_p, sizeof(Tel) * nNewSize));
#endif
          if(pNew)
          {
            m_p = pNew;
            m_allocatedSize = nNewSize;
            m_reportedSize = n;
            r = true;
          }
        }
      }
      return r;
    }

    // returns a const buffer
    const Tel* GetBuffer() const
    {
      return m_p;
    }

    Tel* GetBuffer()
    {
      return m_p;
    }

    Tel& operator [] (size_t index)
    {
      return m_p[index];
    }

    const Tel& operator [] (size_t index) const
    {
      return m_p[index];
    }

    // combination of Alloc(), Lock() and GetLockedBuffer(void)
    // call Unlock() after this!
    Tel* GetBuffer(size_t n)
    {
      Tel* r = 0;
      if(Alloc(n))
      {
          r = m_p;
      }
      return r;
    }

  private:
    size_t m_allocatedSize;
    size_t m_reportedSize;
    Tel* m_p;
    Tel m_StaticBuffer[_StaticBufferSize];
  };
}


#pragma warning(pop)
