//
//
//
//
//

#include "stdafx.hpp"

#include "buffer.hpp"
#include "options.hpp"

CRegistryKey::CRegistryKey()
{
    m_hKey = 0;
    m_sPath.clear();
    m_hRoot = 0;
}


CRegistryKey::~CRegistryKey()
{
    Uninit();
}


bool CRegistryKey::OpenSubKey(const tstd::tstring& sPath, CRegistryKey* p)
{
    return p->Init(m_hKey, sPath);
}


bool CRegistryKey::Delete()
{
    bool r = false;

    RegCloseKey(m_hKey);
    m_hKey = 0;

    if(ERROR_SUCCESS == SHDeleteKey(m_hRoot, m_sPath.c_str()))
    {
        r = true;
    }

    Uninit();

    return r;
}


bool CRegistryKey::Init(HKEY hKey, const tstd::tstring& sPath)
{
    bool r = false;
    Uninit();

    m_hRoot = hKey;
    m_sPath = sPath;

    if(ERROR_SUCCESS == RegCreateKeyEx(hKey, sPath.c_str(), 0, 0, 0, KEY_ALL_ACCESS, 0, &m_hKey, 0))
    {
        r = true;
    }
    return r;
}


void CRegistryKey::Uninit()
{
    if(m_hKey)
    {
        RegCloseKey(m_hKey);
    }
    m_hKey = 0;
    m_sPath.clear();
    m_hRoot = 0;
}


bool CRegistryKey::SetString(const tstd::tstring& Key, const tstd::tstring& Value)
{
    bool r = false;
    if(ERROR_SUCCESS == RegSetValueEx(m_hKey, Key.c_str(), 0, REG_SZ, (const BYTE*)Value.c_str(), (DWORD)sizeof(TCHAR)*(Value.length()+1)))
    {
        r = true;
    }
    return r;
}


bool CRegistryKey::SetBytes(const tstd::tstring& Key, CBuffer<BYTE>& pData, int nElements)
{
    bool r = false;
    const BYTE* pBuf = pData.Lock();
    if(pBuf)
    {
        if(ERROR_SUCCESS == RegSetValueEx(m_hKey, Key.c_str(), 0, REG_BINARY,
            pBuf, pData.GetByteSize(nElements)))
        {
            r = true;
        }

        pData.Unlock();
    }
    return r;
}


bool CRegistryKey::SetDWORD(const tstd::tstring& Key, DWORD dw)
{
    bool r = false;
    if(ERROR_SUCCESS == RegSetValueEx(m_hKey, Key.c_str(), 0, REG_DWORD,
        (const BYTE*)&dw, sizeof(DWORD)))
    {
        r = true;
    }
    return r;
}


bool CRegistryKey::GetString(const tstd::tstring& Key, tstd::tstring& Value)
{
    bool r = false;
    CBuffer<BYTE> b;
    DWORD dwType = 0;

    if(GetRawData(Key, b, &dwType))
    {
        if(dwType == REG_SZ)
        {
            Value = (TCHAR*)b.buf();
            r = true;
        }
    }

    return r;
}


bool CRegistryKey::GetBytes(const tstd::tstring& Key, CBuffer<BYTE>& b)
{
    bool r = false;
    DWORD dwType = 0;

    if(GetRawData(Key, b, &dwType))
    {
        if(dwType == REG_BINARY)
        {
            r = true;
        }
    }

    return r;
}


bool CRegistryKey::GetDWORD(const tstd::tstring& Key, DWORD* pdw)
{
    bool r = false;
    CBuffer<BYTE> b;
    DWORD dwType = 0;

    if(GetRawData(Key, b, &dwType))
    {
        if(dwType == REG_DWORD)
        {
            *pdw = *((DWORD*)b.buf());
            r = true;
        }
    }

    return r;
}


bool CRegistryKey::GetRawData(const tstd::tstring& Key, CBuffer<BYTE>& buf, DWORD* pdwType)
{
    bool r = false;
    BYTE* pBuffer = 0;
    DWORD dwSize = 0;

    // Get the required size and type info.
    if(ERROR_SUCCESS == RegQueryValueEx(m_hKey, Key.c_str(), 0, pdwType, 0, &dwSize))
    {
        // allocte the buffer
        if(buf.Realloc(dwSize))
        {
            pBuffer = buf.Lock();
            // get the data.
            dwSize = buf.GetByteSize(buf.GetSize()); 
            if(ERROR_SUCCESS == RegQueryValueEx(m_hKey, Key.c_str(), 0, 0, pBuffer, &dwSize))
            {
                r = true;
            }

            buf.Unlock();
        }
    }

    return r;
}



CRegistryKeysEnum::CRegistryKeysEnum()
{
    m_hKey = 0;
    m_dwIndex = 0;
    m_pOwner = 0;
}


CRegistryKeysEnum::CRegistryKeysEnum(CRegistryKey* pOwner)
{
    m_hKey = 0;
    m_dwIndex = 0;
    m_pOwner = 0;

    Reset(pOwner);
}


bool CRegistryKeysEnum::Reset(CRegistryKey* pOwner)
{
    m_pOwner = pOwner;
    m_hKey = pOwner->m_hKey;
    m_dwIndex = 0;
    return true;
}


bool CRegistryKeysEnum::Reset()
{
    m_dwIndex = 0;
    return true;
}


#define INITIAL_ALLOC MAX_PATH
bool CRegistryKeysEnum::Next(CRegistryKey& k)
{
    bool r = false;
    FILETIME ft = {0};
    CBuffer<TCHAR> b;
    TCHAR* tszName = 0;
    DWORD dwSize = INITIAL_ALLOC;

    b.Alloc(INITIAL_ALLOC);
    tszName = b.Lock();
    if(tszName)
    {
        if(ERROR_SUCCESS == RegEnumKeyEx(m_hKey, m_dwIndex, tszName, &dwSize, 0, 0, 0, &ft))
        {
            if(dwSize > INITIAL_ALLOC)
            {
                // we didn't get the whole thing... let's make sure we got it this time.
                b.Unlock();
                dwSize = dwSize + 1;
                b.Realloc(dwSize);
                tszName = b.Lock();
                RegEnumKeyEx(m_hKey, m_dwIndex, tszName, &dwSize, 0, 0, 0, &ft);
            }

            m_dwIndex ++;

            k.Init(m_hKey, tstd::tstring(tszName));

            r = true;
        }

        b.Unlock();
        b.Free();
    }

    return r;
}