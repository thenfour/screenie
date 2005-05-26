//
// buffer.hpp - simple buffer used for registry access
// Copyright (c) 2003 Carl Corcoran
//
//

#ifndef SCREENIE_BUFFER_HPP
#define SCREENIE_BUFFER_HPP

// This buffer class also has a granularity mechanism so it will attempt to allocate more than needed
// to reduce the number of OS heap calls.
#define CBUFFER_DEFAULT_GRANULARITY  60

template<typename T>
class CBuffer
{
public:
    CBuffer();
    CBuffer(T* p, int size);
    ~CBuffer();

    bool AssignFrom(CBuffer* pSrc);
    bool AssignFrom(T* p, int size);

    const T* buf();// returns 0 if nothing has been allocated
    T* Lock();// returns 0 if nothing has been allocated.  You can't lock the same object more than once.  It will return 0.
    bool Unlock();// returns false if someone else has it locked - ALWAYS CHECK THIS!
    bool SetGranularity(int n);

    bool Alloc(int nNewSize);
    bool Realloc(int nNewSize);
    bool Free();

    int GetSize();
    int GetElementSize();
    int GetByteSize(int nElements);

private:
    int m_nGranularity;
    bool m_bLocked;
    T* m_sz;// THE BUFFER
    int m_nSize;// Current size of the buffer.
};


template<typename T>
CBuffer<T>::CBuffer(T* p, int size)
{
    m_nGranularity = CBUFFER_DEFAULT_GRANULARITY;
    m_nSize = 0;
    m_sz = 0;
    m_bLocked = false;
    AssignFrom(p, size);
}

template<typename T>
bool CBuffer<T>::AssignFrom(T* p, int size)
{
    bool r = false;
    if(Realloc(size))
    {
        memcpy(this->m_sz, p, size * this->GetByteSize(size));
    }
    return r;
}

template<typename T>
CBuffer<T>::CBuffer()
{
    m_nGranularity = CBUFFER_DEFAULT_GRANULARITY;
    m_nSize = 0;
    m_sz = 0;
    m_bLocked = false;
}

template<typename T>
CBuffer<T>::~CBuffer()
{
    Unlock();
    Free();
}

template<typename T>
bool CBuffer<T>::AssignFrom(CBuffer* pSrc)
{
    bool r = false;
    Free();

    m_nGranularity = pSrc->m_nGranularity;
    if(Alloc(pSrc->m_nSize))
    {
        m_bLocked = pSrc->m_bLocked;
        r = true;
    }

    return r;
}


template<typename T>
bool CBuffer<T>::SetGranularity(int n)
{
    m_nGranularity = n;
}

template<typename T>
const T* CBuffer<T>::buf()
{
    return m_sz;
}

template<typename T>
T* CBuffer<T>::Lock()
{
    if(m_bLocked == true) return 0;

    // Allocate something if we need to.
    if(!m_sz)
    {
        Alloc(1);
        if(m_sz) m_sz[0] = 0;
    }

    m_bLocked = true;
    return m_sz;
}

template<typename T>
bool CBuffer<T>::Unlock()
{
    m_bLocked = false;
    return true;
}

template<typename T>
bool CBuffer<T>::Alloc(int nNewSize)
{
    if(m_bLocked == true) return false;
    if(nNewSize <= m_nSize)
    {
        if(m_sz)
        {
            m_sz[0] = 0;
            return true;
        }
    }

    if(Free() == false) return false;

    // the actual amount to allocate.
    nNewSize = ((nNewSize/m_nGranularity)+1)*m_nGranularity;

    m_sz = (T*)malloc(nNewSize*GetElementSize());

    if(!m_sz)
    {
        m_nSize = 0;
        return false;
    }

    m_sz[0] = 0;
    m_nSize = nNewSize;

    return true;
}

template<typename T>
bool CBuffer<T>::Realloc(int nNewSize)
{
    T* szNew = 0;

    if(m_bLocked == true) return false;
    if(nNewSize <= m_nSize) return true;

    nNewSize = ((nNewSize/m_nGranularity)+1)*m_nGranularity;

    if(!m_sz)
    {
        return Alloc(nNewSize);
    }

    szNew = (T*)realloc(m_sz, nNewSize*GetElementSize());
    if(!szNew)
    {
        return false;
    }

    m_sz = szNew;
    m_nSize = nNewSize;

    return true;
}

template<typename T>
bool CBuffer<T>::Free()
{
    if(m_bLocked == true)
    {
        return false;
    }
    if(m_sz)
    {
        free(m_sz);
        m_sz = 0;
        m_nSize = 0;
    }
    return true;
}

template<typename T>
int CBuffer<T>::GetSize()
{
    return m_nSize;
}


template<typename T>
int CBuffer<T>::GetElementSize()
{
    return sizeof(T);
}


template<typename T>
int CBuffer<T>::GetByteSize(int nElements)
{
    return nElements*GetElementSize();
}

#endif