

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
    VS_FIXEDFILEINFO* ffi = 0;
    UINT ffilen = 0;
    if(!VerQueryValue((const LPVOID)data.GetBuffer(), _T("\\"), (void**)&ffi, &ffilen)) return;

    // verify this.
    m_a = HIWORD(ffi->dwFileVersionMS);
    m_b = LOWORD(ffi->dwFileVersionMS);
    m_c = HIWORD(ffi->dwProductVersionLS);
    m_d = LOWORD(ffi->dwProductVersionLS);
  }

  WORD GetA() const { return m_a; }
  WORD GetB() const { return m_b; }
  WORD GetC() const { return m_c; }
  WORD GetD() const { return m_d; }

private:
  WORD m_a;
  WORD m_b;
  WORD m_c;
  WORD m_d;
};

