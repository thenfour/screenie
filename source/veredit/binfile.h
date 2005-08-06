
#pragma once

#include <string>

using namespace std;

class BinaryFile
{
public:
  BinaryFile(const string& fileName, bool write)
  {
    m_hFile = CreateFile(
      fileName.c_str(),
      write ? GENERIC_WRITE : GENERIC_READ,
      FILE_SHARE_READ,
      0,
      write ? CREATE_ALWAYS : OPEN_EXISTING,
      0, 0
      );
    if(LibCC::IsBadHandle(m_hFile))
    {
      throw exception();
    }
  }

  ~BinaryFile()
  {
    CloseHandle(m_hFile);
  }

  // PODs
  bool WriteZeroBytes(size_t bytes)
  {
    BYTE b = 0;
    for(size_t i = 0; i < bytes; i ++)
    {
      InternalWrite(b);
    }
    return true;
  }
  bool Write(int val)
  {
    return InternalWrite(val);
  }
  bool Write(WORD val)
  {
    return InternalWrite(val);
  }
  bool Write(DWORD val)
  {
    return InternalWrite(val);
  }
  template<typename T>
  bool Write(const T* val, size_t count)
  {
    return InternalWrite(val, count);
  }
  // string
  template<typename T>
  bool Write(const T* val)
  {
    size_t len = LibCC::StringLength(val);
    // write length, then data.
    if(!InternalWrite(val, len))
    {
      return false;
    }
    return InternalWrite((T)0);// null terminator
  }
  template<typename T>
  bool Write(const std::basic_string<T>& val)
  {
    return Write(val.c_str());
  }

  // PODs
  bool Skip(size_t bytes)
  {
    SetFilePointer(m_hFile, (LONG) bytes, 0, FILE_CURRENT);
    return true;
  }
  bool Read(int& val)
  {
    return InternalRead(val);
  }
  bool Read(WORD& val)
  {
    return InternalRead(val);
  }
  bool Read(DWORD& val)
  {
    return InternalRead(val);
  }
  template<typename T>
  bool Read(T* val, size_t count)
  {
    return InternalRead(val, count);
  }
  template<typename T>
  bool Read(std::basic_string<T>& val)
  {
    T ch;
    for(;;)
    {
      if(!Read(&ch, 1))
      {
        return false;
      }
      if(ch == 0)
      {
        break;
      }
      val.push_back(ch);
    }
    return true;
  }

public:

  template<typename T>
  bool InternalWrite(const T* val, size_t count)
  {
    DWORD size = sizeof(T) * static_cast<DWORD>(count);
    DWORD br;
    WriteFile(m_hFile, val, size, &br, 0);
    return br == size;
  }

  template<typename T>
  bool InternalWrite(const T& val)
  {
    return InternalWrite(&val, 1);
  }

  template<typename T>
  bool InternalRead(T* val, size_t count)
  {
    DWORD size = sizeof(T) * static_cast<DWORD>(count);
    DWORD br;
    ReadFile(m_hFile, val, size, &br, 0);
    return br == size;
  }

  template<typename T>
  bool InternalRead(T& val)
  {
    return InternalRead(&val, 1);
  }

  HANDLE m_hFile;
};


class BinaryMemory
{
public:
  BinaryMemory(void* p, size_t bytes)
  {
    m_buffer.Alloc(bytes);
    memcpy(m_buffer.GetBuffer(), p, bytes);
    m_filePointer = 0;
  }

  BinaryMemory()
  {
    m_filePointer = 0;
  }

  // PODs
  bool WriteZeroBytes(size_t bytes)
  {
    BYTE b = 0;
    for(size_t i = 0; i < bytes; i ++)
    {
      InternalWrite(b);
    }
    return true;
  }
  bool Write(int val)
  {
    return InternalWrite(val);
  }
  bool Write(WORD val)
  {
    return InternalWrite(val);
  }
  bool Write(DWORD val)
  {
    return InternalWrite(val);
  }
  template<typename T>
  bool Write(const T* val, size_t count)
  {
    return InternalWrite(val, count);
  }
  // string
  template<typename T>
  bool Write(const T* val)
  {
    size_t len = LibCC::StringLength(val);
    // write length, then data.
    if(!InternalWrite(val, len))
    {
      return false;
    }
    return InternalWrite((T)0);// null terminator
  }
  template<typename T>
  bool Write(const std::basic_string<T>& val)
  {
    return Write(val.c_str());
  }

  // PODs
  bool Skip(size_t bytes)
  {
    m_filePointer += bytes;
    return true;
  }
  bool Read(int& val)
  {
    return InternalRead(val);
  }
  bool Read(WORD& val)
  {
    return InternalRead(val);
  }
  bool Read(DWORD& val)
  {
    return InternalRead(val);
  }
  template<typename T>
  bool Read(T* val, size_t count)
  {
    return InternalRead(val, count);
  }
  template<typename T>
  bool Read(std::basic_string<T>& val)
  {
    T ch;
    for(;;)
    {
      if(!Read(&ch, 1))
      {
        return false;
      }
      if(ch == 0)
      {
        break;
      }
      val.push_back(ch);
    }
    return true;
  }

  const BYTE* GetBuffer() const
  {
    return m_buffer.GetBuffer();
  }

  size_t GetSize() const
  {
    return m_buffer.Size();
  }

private:

  template<typename T>
  bool InternalWrite(const T* val, size_t count)
  {
    // reserve memory
    size_t finalSize = m_filePointer + (sizeof(T) * count);
    if(m_buffer.Size() < finalSize)
    {
      m_buffer.Alloc(finalSize);
    }
    BYTE* p = m_buffer.GetBuffer() + m_filePointer;
    for(size_t i = 0; i < count; i ++)
    {
      memcpy(p, val + i, sizeof(T));
      p += sizeof(T);
      m_filePointer += sizeof(T);
    }
    return true;
  }

  template<typename T>
  bool InternalRead(T* val, size_t count)
  {
    BYTE* p = m_buffer.GetBuffer() + m_filePointer;
    for(size_t i = 0; i < count; i ++)
    {
      memcpy(val + i, p, sizeof(T));
      p += sizeof(T);
      m_filePointer += sizeof(T);
    }
    return true;
  }

  template<typename T>
  bool InternalWrite(const T& val)
  {
    return InternalWrite(&val, 1);
  }

  template<typename T>
  bool InternalRead(T& val)
  {
    return InternalRead(&val, 1);
  }

  LibCC::Blob<BYTE> m_buffer;
  size_t m_filePointer;
};
