//
//
//
//
//

#ifndef SCREENIE_OPTIONS_HPP
#define SCREENIE_OPTIONS_HPP

#include "tstdlib/tstring.hpp"
#include "buffer.hpp"

class CRegistryKey
{
    friend class CRegistryKeysEnum;
public:
    CRegistryKey();
    ~CRegistryKey();

    bool Delete();
    bool Init(HKEY hKey, const tstd::tstring& sPath);
    void Uninit();

    bool OpenSubKey(const tstd::tstring& sPath, CRegistryKey* p);

    // Key is something like "Window Placement" and value is a string
    bool SetString(const tstd::tstring& Key, const tstd::tstring& Value);
    bool SetBytes(const tstd::tstring& Key, CBuffer<BYTE>& pData, int nElements);
    bool SetDWORD(const tstd::tstring& Key, DWORD dw);

    bool GetString(const tstd::tstring& Key, tstd::tstring& Value);
    bool GetBytes(const tstd::tstring& Key, CBuffer<BYTE>& b);
    bool GetDWORD(const tstd::tstring& Key, DWORD* pdw);

private:
    bool GetRawData(const tstd::tstring& Key, CBuffer<BYTE>& buf, DWORD* pdwType);

    HKEY m_hKey;

    // values that were used to initialize this thing.
    tstd::tstring m_sPath;
    HKEY m_hRoot;
};

class CRegistryKeysEnum
{
public:
    CRegistryKeysEnum();
    CRegistryKeysEnum(CRegistryKey*);

    bool Reset(CRegistryKey*);
    bool Reset();
    bool Next(CRegistryKey&);
private:
    DWORD m_dwIndex;
    HKEY m_hKey;

    CRegistryKey* m_pOwner;
};

#endif