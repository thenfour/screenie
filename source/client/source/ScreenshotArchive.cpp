#include "StdAfx.hpp"
#include "ScreenshotArchive.hpp"
#include "codec.hpp"
#include "libcc\blob.h"
#include "image.hpp"

class BlobStream : public IStream
{
	LibCC::Blob<BYTE> m_blob;
	size_t m_cursor;
public:
	BlobStream() :
		m_cursor(0)
	{
	}

	void* GetBuffer() const 
	{
		return (void*)m_blob.GetBuffer();
	}
	int GetLength() const
	{
		return m_blob.Size();
	}

	// IUnknown
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject)
	{
		if(riid == IID_IUnknown)
		{
			*ppvObject = (IUnknown*)this; 
			return S_OK;
		}
		if(riid == IID_IStream)
		{
			*ppvObject = (IStream*)this; 
			return S_OK;
		}
		return E_NOINTERFACE;
	}
  ULONG STDMETHODCALLTYPE AddRef( void)
	{
		return 1;
	}
  ULONG STDMETHODCALLTYPE Release( void)
	{
		return 1;
	}

	// IStream
  HRESULT STDMETHODCALLTYPE Read(void *pv, ULONG cb, ULONG *pcbRead)
	{
		if(m_cursor >= m_blob.Size())
		{
			return STG_E_INVALIDPOINTER;
		}
		size_t sizeLeft = m_blob.Size() - m_cursor;
		size_t toRead = min(cb, sizeLeft);
		memcpy(pv, m_blob.GetBuffer() + m_cursor, toRead);
		m_cursor += toRead;
		if(pcbRead)
			*pcbRead = toRead;
		return toRead == cb ? S_OK : S_FALSE;
	}
	  
	HRESULT STDMETHODCALLTYPE Write(const void *pv, ULONG cb, ULONG *pcbWritten)
	{
		if(m_cursor > m_blob.Size())
		{
			return STG_E_INVALIDPOINTER;
		}
		size_t cursorAfter = m_cursor + cb;
		if(cursorAfter > m_blob.Size())
		{
			m_blob.Alloc(cursorAfter);
		}
		memcpy(m_blob.GetBuffer() + m_cursor, pv, cb);
		*pcbWritten = cb;
		m_cursor = cursorAfter;
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE Seek( LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
	{
		if(dlibMove.HighPart > 0)
		{
			return STG_E_INVALIDPOINTER;
		}

		switch(dwOrigin)
		{
		case STREAM_SEEK_CUR:
			m_cursor += (int)dlibMove.LowPart;
			break;
		case STREAM_SEEK_END:
			m_cursor = m_blob.Size() + (int)dlibMove.LowPart;
			break;
		case STREAM_SEEK_SET:
			m_cursor = dlibMove.LowPart;
			break;
		default:
			return STG_E_INVALIDFUNCTION;
		}
		if(m_cursor > m_blob.Size())
		{
			return STG_E_INVALIDPOINTER;
		}
		
		if(plibNewPosition)
			plibNewPosition->QuadPart = 0;

		return S_OK;
	}
  
  HRESULT STDMETHODCALLTYPE SetSize( ULARGE_INTEGER libNewSize)
	{
		if(libNewSize.HighPart > 0) return STG_E_MEDIUMFULL;
		m_blob.Alloc(libNewSize.LowPart);
		return S_OK;
	}
  
  HRESULT STDMETHODCALLTYPE CopyTo(  IStream *pstm, ULARGE_INTEGER cb,ULARGE_INTEGER *pcbRead,ULARGE_INTEGER *pcbWritten)
	{
		if(m_cursor > m_blob.Size())
		{
			return STG_E_INVALIDPOINTER;
		}
		if(cb.HighPart > 0) return STG_E_MEDIUMFULL;

		ULONG bw = 0;
		HRESULT hr = pstm->Write(m_blob.GetBuffer() + m_cursor, cb.LowPart, &bw);
		if(pcbWritten)
		{
			pcbWritten->HighPart = 0;
			pcbWritten->LowPart = bw;
		}
		return hr;
	}
  
  HRESULT STDMETHODCALLTYPE Commit( DWORD grfCommitFlags)
	{
		return S_OK;
	}
  
  HRESULT STDMETHODCALLTYPE Revert( void)
	{
		return E_NOTIMPL;
	}
  
  HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
	{
		return E_NOTIMPL;
	}
  
  HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
	{
		return E_NOTIMPL;
	}
  
  HRESULT STDMETHODCALLTYPE Stat(  STATSTG *pstatstg,DWORD grfStatFlag)
	{
		if(grfStatFlag & STATFLAG_NONAME)
		{
			// uh i should really set more stuff here, but i don't need to at the moment.
			pstatstg->cbSize.QuadPart = m_blob.Size();
			return S_OK;
		}
		return E_NOTIMPL;
	}
  
  HRESULT STDMETHODCALLTYPE Clone( IStream **ppstm)
	{
		return E_NOTIMPL;
	}
};

std::wstring QuoteSql(const std::wstring& in)
{
	std::wstring ret;
	ret.reserve(in.length());
	for(std::wstring::const_iterator it = in.begin(); it != in.end(); ++ it)
	{
		if(*it == L'\'')
		{
			ret.push_back(L'\'');
			ret.push_back(L'\'');
		}
		else
		{
			ret.push_back(*it);
		}
	}
	return ret;
}

util::shared_ptr<Gdiplus::Bitmap> ScreenshotArchive::Screenshot::RetrieveImage()
{
	Gdiplus::Bitmap* p = 0;
	return util::shared_ptr<Gdiplus::Bitmap>(p);
}

std::vector<ScreenshotArchive::Event> ScreenshotArchive::Screenshot::RetreiveEvents()
{
	std::vector<ScreenshotArchive::Event> ret;
	return ret;
}

ScreenshotArchive::ScreenshotArchive(ScreenshotOptions& opt) :
	m_options(opt)
{
}

ScreenshotArchive::~ScreenshotArchive()
{
}

std::vector<ScreenshotArchive::Screenshot> ScreenshotArchive::RetreiveScreenshots()
{
	std::vector<ScreenshotArchive::Screenshot> ret;
	return ret;
}

std::wstring ScreenshotArchive::GetDBFilename() const
{
	// generate the filename.
	std::wstring path = m_options.GetConfigPath();
	path = LibCC::PathRemoveFilename(path);
	LibCC::PathAppendX(path, L"screenie.db");
	return path;
}

// TODO: Optimize this.
bool ScreenshotArchive::EnsureDatabaseHasSpace(size_t extra)
{
	size_t extraMB = extra;
	std::wstring filename = GetDBFilename();
	
	if(extraMB > (size_t)m_options.ArchiveLimit())
		return false;// there's no possible way you could store this amount of data.

	do
	{
		HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
		if(LibCC::IsBadHandle(hFile))
			return true;// probably just not found.
		DWORD sizeMB = GetFileSize(hFile, 0);
		CloseHandle(hFile);
		if((sizeMB + extraMB) < (size_t)m_options.ArchiveLimit())
		{
			return true;
		}

		try
		{
			// delete some stuff.
			sqlite3x::sqlite3_connection conn;
			if(!OpenDatabase(conn))
				return true;

			int count = conn.executeint("select count(*) from screenshots");
			if(count == 0)// there are no records, but we know that there's no space. nothing we can do.
				return false;

			std::wstring screenshotDate = conn.executestring16("select min([date]) from screenshots");
			int screenshotID = conn.executeint(LibCC::Format("select [id] from screenshots where [date] = '%'").s(screenshotDate).Str());
			conn.executenonquery(LibCC::Format("delete from Events where screenshotID = %")(screenshotID).Str());
			conn.executenonquery(LibCC::Format("delete from Screenshots where [id] = %")(screenshotID).Str());

			// now vacuum
			conn.executenonquery("vacuum");
		}
		catch(sqlite3x::database_error& er)
		{
			LibCC::g_pLog->Message(LibCC::Format("Database exception while shrinking the database. %").qs(er.what()));
			return false;
		}
	}
	while(true);

	// unreachable code.
	return true;
}

int ScreenshotArchive::RegisterNewScreenshot(util::shared_ptr<Gdiplus::Bitmap> bmp)
{
	int ret = 0;
	// save a lossless PNG file to a memory stream
	BlobStream stream;
	ImageCodecsEnum codecs;
	Gdiplus::ImageCodecInfo* codecInfo = codecs.GetCodecByMimeType(_T("image/png"));
	Gdiplus::Status status = bmp->Save(&stream, &codecInfo->Clsid);
	if(status != Gdiplus::Ok)
		return 0;

	try
	{
		// make sure there is space in the db file.
		if(!EnsureDatabaseHasSpace(stream.GetLength()))
			return 0;

		// and save to the database.
		sqlite3x::sqlite3_connection conn;
		if(!OpenDatabase(conn)) return 0;
		sqlite3x::sqlite3_command cmd(conn, "insert into Screenshots ([bitmapData], [date]) values (?, strftime('%Y-%m-%dT%H:%M:%f', 'now'))");
		cmd.bind(1, stream.GetBuffer(), stream.GetLength());

		//LARGE_INTEGER li;
		//li.QuadPart = 0;
		//stream.Seek(li, STREAM_SEEK_SET, 0);
		//Gdiplus::Bitmap* x = new Gdiplus::Bitmap(&stream);
		//DumpBitmap(*x);
		//delete x;

		cmd.executenonquery();
		ret = (int)conn.insertid();
	}
	catch(sqlite3x::database_error& er)
	{
		LibCC::g_pLog->Message(LibCC::Format("Database exception while inserting screenshot data. %").qs(er.what()));
	}

	return ret;
}

int ScreenshotArchive::RegisterNewEvent(int screenshotID,
	MessageIcon icon, MessageType messageType,
	const std::wstring& destinationName, const std::wstring& messageText, const std::wstring& data1)
{
	int ret = 0;
	try
	{
		sqlite3x::sqlite3_connection conn;
		if(!OpenDatabase(conn)) return 0;
		std::wstring sql = LibCC::Format("insert into Events (screenshotID, icon, messageType, destinationName, messageText, data1, [date]) values "
			"(%, %, %, '%', '%', '%', strftime('^%Y-^%m-^%dT^%H:^%M:^%f', 'now'))")
			(screenshotID)
			((int)icon)
			((int)messageType)
			(QuoteSql(destinationName))
			(QuoteSql(messageText))
			(QuoteSql(data1))
			.Str();

		conn.executenonquery(sql);

		ret = (int)conn.insertid();
	}
	catch(sqlite3x::database_error& er)
	{
		LibCC::g_pLog->Message(LibCC::Format("Database exception while inserting an event. %").qs(er.what()));
	}
	return ret;
}

bool ScreenshotArchive::OpenDatabase(sqlite3x::sqlite3_connection& out)
{
	std::wstring path = GetDBFilename(); 
	bool openSuccessful = false;

	if(PathFileExists(path.c_str()))
	{
		try
		{
			out.open(path.c_str());
			sqlite3x::sqlite3_command cmd(out, "select [intValue] from Settings where [Name] like 'Version'");
			int i = cmd.executeint();
			if(i == DatabaseVersion)
			{
				openSuccessful = true;
			}
			else
			{
				LibCC::g_pLog->Message(LibCC::Format("Database % is the wrong version (it's %, but I expected %)").qs(path).i(i).i(DatabaseVersion));
				out.close();
			}
		}
		catch(sqlite3x::database_error& er)
		{
			// no big deal; the database probable doesn't exist.
			LibCC::g_pLog->Message(LibCC::Format("Database exception while opening %: %; trying to create a new database.").qs(path).qs(er.what()));
		}
	}

	if(!openSuccessful)
	{
		try
		{
			DeleteFile(path.c_str());
			out.open(path.c_str());
			// create schema
			out.executenonquery("create table Settings ([Name] text, [intValue] int, [stringValue] text)");
			out.executenonquery("create table Screenshots ([id] INTEGER PRIMARY KEY AUTOINCREMENT, [bitmapData] blob, [date] text)");
			out.executenonquery("create table Events ([id] INTEGER PRIMARY KEY AUTOINCREMENT, screenshotID int, icon int, messageType int, destinationName text, messageText text, data1 text, [date] text)");
			out.executenonquery(LibCC::Format("insert into Settings ([Name], [intValue]) values ('Version', %)")(DatabaseVersion).Str());
			openSuccessful = true;
		}
		catch(sqlite3x::database_error& er)
		{
			// no big deal; the database probable doesn't exist.
			LibCC::g_pLog->Message(LibCC::Format("Error while setting up new database (%). Sorry, man, archiving not available.").qs(er.what()));
		}
	}

	if(!openSuccessful)
	{
		return false;
	}

	return true;
}
