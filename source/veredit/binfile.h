
#pragma once

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
  Result WriteZeroBytes(size_t bytes)
  {
    Result ret;
    BYTE b = 0;
    for(size_t i = 0; i < bytes; i ++)
    {
      if((ret = InternalWrite(b)).Failed())
      {
        return ret.Prepend("WriteZeroBytes failed. ");
      }
    }
    return ret.Succeed();
  }
  Result Write(int val)
  {
    return InternalWrite(val);
  }
  Result Write(WORD val)
  {
    return InternalWrite(val);
  }
  Result Write(DWORD val)
  {
    return InternalWrite(val);
  }
  template<typename T>
  Result Write(const T* val, size_t count)
  {
    return InternalWrite(val, count);
  }
  // string
  template<typename T>
  Result Write(const T* val)
  {
    Result ret;
    size_t len = LibCC::StringLength(val);
    // write length, then data.
    if(!(ret = InternalWrite(val, len)))
    {
      return ret;
    }
    return InternalWrite((T)0);// null terminator
  }
  template<typename T>
  Result Write(const std::basic_string<T>& val)
  {
    return Write(val.c_str());
  }

  // PODs
  Result Skip(size_t bytes)
  {
    SetFilePointer(m_hFile, (LONG) bytes, 0, FILE_CURRENT);
    return Result(true);
  }
  Result Read(int& val)
  {
    return InternalRead(val);
  }
  Result Read(WORD& val)
  {
    return InternalRead(val);
  }
  Result Read(DWORD& val)
  {
    return InternalRead(val);
  }
  template<typename T>
  Result Read(T* val, size_t count)
  {
    return InternalRead(val, count);
  }
  template<typename T>
  Result Read(std::basic_string<T>& val)
  {
    Result ret;
    T ch;
    for(;;)
    {
      if(!(ret = Read(&ch, 1)))
      {
        return ret;
      }
      if(ch == 0)
      {
        break;
      }
      val.push_back(ch);
    }
    return ret.Succeed();
  }

private:
  template<typename T>
  Result InternalWrite(const T* val, size_t count)
  {
    DWORD size = sizeof(T) * static_cast<DWORD>(count);
    DWORD br;
    if(0 == WriteFile(m_hFile, val, size, &br, 0))
    {
      return Result(false, Format("Write failed, gle=%").gle(GetLastError()));
    }
    return Result(br == size);
  }

  template<typename T>
  Result InternalWrite(const T& val)
  {
    return InternalWrite(&val, 1);
  }

  template<typename T>
  Result InternalRead(T* val, size_t count)
  {
    DWORD size = sizeof(T) * static_cast<DWORD>(count);
    DWORD br;
    if(0 == ReadFile(m_hFile, val, size, &br, 0))
    {
      return Result(false, Format("Write failed, gle=%").gle(GetLastError()));
    }
    return Result(br == size);
  }

  template<typename T>
  Result InternalRead(T& val)
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
  Result WriteZeroBytes(size_t bytes)
  {
    Result ret;
    BYTE b = 0;
    for(size_t i = 0; i < bytes; i ++)
    {
      if((ret = InternalWrite(b)).Failed())
      {
        return ret.Prepend("WriteZeroBytes failed. ");
      }
    }
    return ret.Succeed();
  }
  Result Write(int val)
  {
    return InternalWrite(val);
  }
  Result Write(WORD val)
  {
    return InternalWrite(val);
  }
  Result Write(DWORD val)
  {
    return InternalWrite(val);
  }
  template<typename T>
  Result Write(const T* val, size_t count)
  {
    return InternalWrite(val, count);
  }
  // string
  template<typename T>
  Result Write(const T* val)
  {
    Result ret;
    size_t len = LibCC::StringLength(val);
    // write length, then data.
    if(!(ret = InternalWrite(val, len)))
    {
      return ret;
    }
    return InternalWrite((T)0);// null terminator
  }
  template<typename T>
  Result Write(const std::basic_string<T>& val)
  {
    return Write(val.c_str());
  }

  // PODs
  Result Skip(size_t bytes)
  {
    m_filePointer += bytes;
    return Result(true);
  }
  Result Read(int& val)
  {
    return InternalRead(val);
  }
  Result Read(WORD& val)
  {
    return InternalRead(val);
  }
  Result Read(DWORD& val)
  {
    return InternalRead(val);
  }
  template<typename T>
  Result Read(T* val, size_t count)
  {
    return InternalRead(val, count);
  }
  template<typename T>
  Result Read(std::basic_string<T>& val)
  {
    Result ret;
    T ch;
    for(;;)
    {
      if(!(ret = Read(&ch, 1)))
      {
        return ret;
      }
      if(ch == 0)
      {
        break;
      }
      val.push_back(ch);
    }
    return ret.Succeed();
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
  Result InternalWrite(const T* val, size_t count)
  {
    // reserve memory
    size_t finalSize = m_filePointer + (sizeof(T) * count);
    if(m_buffer.Size() < finalSize)
    {
      if(!m_buffer.Alloc(finalSize))
      {
        return Result(false, "Failed to allocate memory for buffered write operation");
      }
    }
    BYTE* p = m_buffer.GetBuffer() + m_filePointer;
    for(size_t i = 0; i < count; i ++)
    {
      memcpy(p, val + i, sizeof(T));
      p += sizeof(T);
      m_filePointer += sizeof(T);
    }
    return Result(true);
  }

  template<typename T>
  Result InternalRead(T* val, size_t count)
  {
    BYTE* p = m_buffer.GetBuffer() + m_filePointer;
    for(size_t i = 0; i < count; i ++)
    {
      memcpy(val + i, p, sizeof(T));
      p += sizeof(T);
      m_filePointer += sizeof(T);
    }
    return Result(true);
  }

  template<typename T>
  Result InternalWrite(const T& val)
  {
    return InternalWrite(&val, 1);
  }

  template<typename T>
  Result InternalRead(T& val)
  {
    return InternalRead(&val, 1);
  }

  LibCC::Blob<BYTE> m_buffer;
  size_t m_filePointer;
};
