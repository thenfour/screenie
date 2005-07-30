

#pragma once

class Version
{
public:
  Version() :
    m_a(0),
    m_b(0),
    m_c(0),
    m_d(0)
  {
  }

  void FromFile(LPCTSTR file)
  {
    DWORD wtf;
    DWORD size;
    size = GetFileVersionInfoSize(file, &wtf);
    if(!size) return;

    LibCC::Blob<BYTE> data;
    if(!data.Alloc(size+1)) return;// why +1 ?  just for fun i guess.... no good reason.
    if(!GetFileVersionInfo(file, 0, size, data.GetBuffer())) return;

    // fixed info (main shit)
    VS_FIXEDFILEINFO* ffi = 0;
    UINT ffilen = 0;
    if(!VerQueryValue(data.GetBuffer(), _T("\\"), (void**)&ffi, &ffilen)) return;
    m_a = HIWORD(ffi->dwFileVersionMS);
    m_b = LOWORD(ffi->dwFileVersionMS);
    m_c = HIWORD(ffi->dwProductVersionLS);
    m_d = LOWORD(ffi->dwProductVersionLS);

    // registrant
    PCSTR pStr;
    if(!VerQueryValue(data.GetBuffer(), _T("\\StringFileInfo\\040904b0\\RegisteredTo"), (void**)&pStr, &ffilen)) return;
    m_registrant = pStr;

    // copyright
    if(!VerQueryValue(data.GetBuffer(), _T("\\StringFileInfo\\040904b0\\LegalCopyright"), (void**)&pStr, &ffilen)) return;
    m_copyright = pStr;
  }

  WORD GetA() const { return m_a; }
  WORD GetB() const { return m_b; }
  WORD GetC() const { return m_c; }
  WORD GetD() const { return m_d; }
  std::string GetRegistrant() const { return m_registrant; }
  std::string GetCopyright() const { return m_copyright; }

private:
  std::string m_registrant;
  std::string m_copyright;

  WORD m_a;
  WORD m_b;
  WORD m_c;
  WORD m_d;
};

